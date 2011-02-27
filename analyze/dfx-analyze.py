#!/usr/bin/env python

import sys,os,getopt
from PIL import Image, ImageDraw

# ----- constants -----

marker_index    = 0x01
marker_overflow = 0x02
marker_last     = 0x02

# signals shorter this level are accounted to next sample
noise_level = 24

# timing
one_us   = 24   # 1us is approx. 24 timer ticks

# block len of an amiga block (without sync) in bits
# sync + (header + data) = sync + block
amiga_data_bit_len  = 1024 * 8
amiga_hdr_bit_len   = 56 * 8 
amiga_block_bit_len = amiga_data_bit_len + amiga_hdr_bit_len
amiga_sync_bit_len  = 8 * 8 # AAAA AAAA 4489 4489
amiga_sync_block_bit_len = amiga_block_bit_len + amiga_sync_bit_len

# 
# MFM
#
# a bit cell is 2us size
#   _
# _| |_|
#   1 0 (1  <- four us
#   _
# -| |__|
#   1 0 0 (1 <- six us
#   _
# _| |___|
#   1 0 0 0 (1 <- eight us

four_us  = 4 * one_us
six_us   = 6 * one_us
eight_us = 8 * one_us

# define borders
b0 = four_us - one_us
b1 = six_us - one_us
b2 = eight_us - one_us
b3 = eight_us + one_us

cellcode = ('l','h','2','3','4')
cellcode_to_bits = {
    'l' : '',
    'h' : '',
    '2' : '10',
    '3' : '100',
    '4' : '1000'
}

# ----- input raw track data -----

def read_track_data(name):
    # read data    
    f=open(name,'rb')
    data=f.read()
    f.close();
    return data

def check_overflow(data):
    cnt = 0;
    for d in data:
        t = ord(d)
        if t == marker_overflow:
            cnt += 1
    return cnt
        
def extract_index(data):
    pos = 0
    index_pos=[]
    new_data = ""
    for d in data:
        val = ord(d)
        if val == marker_index:
            index_pos.append(pos)
        else:
            pos += 1
            new_data += d
            
    index=[]
    for i in xrange(len(index_pos)-1):
        index_len = index_pos[i+1]-index_pos[i]
        index.append( (index_pos[i],index_len) )
    return (new_data,index)

# ----- transform time sample to bit cells -----

def filter_spikes(data):
    result = ""
    short_time = 0
    num_merged = 0
    for a in data:
        t = ord(a)
        if t < marker_last:
            # keep markers
            result += chr(t)        
        elif t <= noise_level:
            # short spike -> remember for next sample
            short_time += t
            num_merged += 1
        else:
            # normal sample
            # was a spike? -> add it
            if short_time > 0:
                t += short_time
                short_time = 0
            result += chr(t)
    return (result, num_merged)

def filter_pll(data):
    # the initial sample period we assume
    req_period = one_us * 2.0
    period = req_period
    # step for adjustment
    step = period / 100.0
    # window size: a quarter period
    win_size = 2.0
    # the inital window size
    wt = one_us / win_size

    # the initial accumulated delta
    t = 0
    
    result = ""
    sum_error = 0
    count = 0
    init = True
    
    for a in data:
        v = ord(a)
        if v <= marker_last:
            result += a
            continue
    
        t += v

        # calc distance and num cells of current delta            
        nc = divmod(t, period)
        slots = int(nc[0])
        dist = nc[1]
        dist_frac = dist / wt
        #print "t=",t,"period=",period,"wt=",wt,wt*3,"-> slots=",slots,"dist=",dist,"dist_frac=",dist_frac
        
        if dist > 0:
            # not optimal sync
            if dist <= wt:
                # near optimal position
                period += step
                wt = period / win_size
                out = req_period * slots
                result += chr(out)
                t = 0
                #print "  near: period=",period,"wt=",wt,"->",out
                sum_error += dist_frac
            else:
                # near next optimal position
                period -= step
                wt = period / win_size
                out = req_period * (slots+1)
                result += chr(out)
                t = 0
                #print "  far: period=",period,"wt=",wt,"->",out
                sum_error += win_size - dist_frac
        else:
            out = req_period * slots
            t = 0
            #print "  ok: ",out
            result += chr(out)
        count += 1
            
    return (result, sum_error/count)

def time_to_cellcode_num(t):
    if t < b0:
        return 0 # weak short
    elif t > b3:
        return 1 # weak long
    elif t <= b1:
        return 2 # 2 cells
    elif t <= b2:
        return 3 # 3 cells
    elif t <= b3:
        return 4 # 4 cells

def time_to_cellcode(t):
    return cellcode[time_to_cellcode_num(t)]

def all_times_to_cellcodes(data):
    # now decode data
    result = ""
    for a in data:
        d = ord(a)
        result += time_to_cellcode(d)
    return result

def all_cellcodes_to_bits(data):
    result = ""
    for a in data:
        result += cellcode_to_bits[a]
    return result

# give some timing distribution
def calc_time_stats(data):
    # time sample distribution
    t_stat = []
    for a in xrange(256):
        t_stat.append(0)
    for a in data:
        t = ord(a)
        t_stat[t] += 1
    return t_stat
            
def print_time_stats(t_stat):
    print "--- time statistics ---"
    for a in xrange(256):
        if a == b0 or a == b1 or a == b2 or a == b3:
            print "   ",hex(a),"----"
        if t_stat[a] > 0:
            print "   ",hex(a),hex(t_stat[a])

def calc_cell_stats(data):
    stat = []
    for a in xrange(5):
        stat.append(0)
    for a in data:
        t = ord(a)
        celltype = time_to_cellcode_num(t)
        stat[celltype] += 1
    return stat
    
def print_cell_stats(stat):
    print "--- bitcell statistics ---"
    names = ( 'lo:','hi:','2c:','3c:','4c:')
    for a in xrange(5):
        print "   ",names[a],stat[a]

# ----- cut a raw track into amiga raw blocks with sync detection ----

def amiga_sync():
    bin_a="1010"
    bin_4="0100"
    bin_8="1000"
    bin_9="1001"
    code = bin_4 + bin_4 + bin_8 + bin_9
    sync = bin_a * 8 + code * 2 
    return sync

def cut_by_sync(data,sync=amiga_sync()):
    # skip first bit of sync as this clock bit might change depending on preceeding data
    sync = sync[1:]

    pos = 0
    sync_pos = []
    sync_len = len(sync)
    while True:
        pos = data.find(sync,pos+sync_len)
        if pos != -1:
            sync_pos.append(pos)
        else:
            break
    
    result = []
    for s in xrange(len(sync_pos)-1):
        begin = sync_pos[s] + sync_len
        end   = sync_pos[s+1]
        size  = end - begin - 1 # correct stipped bit above
        result.append( (begin,size) )
    return result

def analyze_chunk_size(raw_blks):
    result = {}
    for b in raw_blks:
        begin = b[0]
        size  = b[1]
        
        # store by size   
        if result.has_key(size):
            result[size] += 1
        else:
            result[size] = 1
    return result

# ----- decoding MFM amiga blocks -----

def decode_bin(data):
    result = 0
    for a in data:
        result *= 2
        if a == '1':
            result |= 1
    return result

def decode_mfm(data):
    result = ""
    even = False
    for a in data:
        if even:
            result += a
        even = not even
    return result

def decode_amiga_mfm_dword(data):
    assert(len(data)==64)
    odd_bits  = decode_mfm(data[0:32])
    even_bits = decode_mfm(data[32:64])
    mix = ""
    for i in xrange(16):
        mix += odd_bits[i] + even_bits[i]
    return decode_bin(mix)

def decode_amiga_mfm_data_part(data):
    size = 512 * 8
    assert(len(data)==size*2)
    odd_bits  = decode_mfm(data[0:size])
    even_bits = decode_mfm(data[size:2*size])
    mix = ""
    for i in xrange(size/2):
        mix += odd_bits[i] + even_bits[i]
    bytes = ""
    off = 0
    for i in xrange(512):
        bytes += chr(decode_bin(mix[off:off+8]))
        off += 8
    return bytes

def decode_amiga_block(data, gap_bits, blk_offset_in_bits):
    assert(len(data) == amiga_block_bit_len)
    
    info = decode_amiga_mfm_dword(data[0:64])
    # info:
    # b format = 0xff
    # b track no. 1..80
    # b sector no. 0..11
    # b sectors to gap (including own!)
    format = int((info >> 24) & 0xff)
    track  = int((info >> 16) & 0xff)
    sector = int((info >> 8) & 0xff)
    sec_to_gap = int( info & 0xff)
    
    # unused part
    off = 64
    unused = []
    for i in xrange(4):
        unused.append(decode_amiga_mfm_dword(data[off:off+64]))
        off += 64
    
    # block and data check sums
    blk_check = decode_amiga_mfm_dword(data[off:off+64])
    off += 64
    data_check = decode_amiga_mfm_dword(data[off:off+64])
    off += 64
    
    # store begin of data
    data_off = off
    
    # check block: xor raw mfm dwords
    my_blk_check = 0
    off = 0
    for i in xrange(10):
        raw_data = decode_bin(data[off:off+32])
        off += 32
        my_blk_check ^= raw_data
    my_blk_check &= 0x55555555
    blk_ok = my_blk_check == blk_check

    # check data: xor raw data dwords
    my_data_check = 0
    off = data_off
    for i in xrange(256):
        raw_data = decode_bin(data[off:off+32])
        off += 32
        my_data_check ^= raw_data
    my_data_check &= 0x55555555
    data_ok = my_data_check == data_check
    
    # data block
    data_block = decode_amiga_mfm_data_part(data[data_off:data_off+1024*8])
    
    return [blk_ok, data_ok, gap_bits,
            format,track,sector,sec_to_gap,
            unused,
            hex(blk_check),hex(data_check), 
            blk_offset_in_bits]

def decode_all_amiga_blocks(data,raw_blks):
    blk_list = []
    for raw_blk in raw_blks:
        off  = raw_blk[0]
        size = raw_blk[1]

        # bits too few or too many
        gap_bits = size - amiga_block_bit_len

        blk_data = data[off:off + amiga_block_bit_len]
        if(len(blk_data) == amiga_block_bit_len):
            blk = decode_amiga_block(blk_data, gap_bits, off)
            blk.append(amiga_sync())
            blk_list.append( blk )
    return blk_list

def assign_blocks(blk_list):
    blk_tab = []
    for i in xrange(11):
        blk_tab.append([[],[]])

    for blk in blk_list:
        sector = blk[5]
        if blk[0] and blk[1]:
            blk_tab[sector][0].append(blk)
        elif blk[0]:
            blk_tab[sector][1].append(blk)

    return blk_tab

def analyze_blocks(blks):
    blk_total = len(blks)
    blk_hdr_ok = 0
    blk_dat_ok = 0
    track_gap = []
    long_sectors = []
    short_list = []
    long_list = []
    track_tab = {}
    for b in blks:
        hdr_ok  = b[0]
        dat_ok  = b[1]
        bit_gap = b[2]
        track   = b[4]
        sector  = b[5]
        gap_sectors = b[6]
        
        # judge bit gap
        if gap_sectors == 1:
            # track gap
            track_gap.append(bit_gap)
        else:
            if bit_gap < 0:
                short_list.append((sector,bit_gap))
            elif bit_gap > 0:
                long_sectors.append(b)
                long_list.append((sector,bit_gap))
        
        # account check sums
        if hdr_ok:
            blk_hdr_ok += 1
        if dat_ok:
            blk_dat_ok += 1
            
        # store track number
        if hdr_ok:
            if track_tab.has_key(track):
                track_tab[track] += 1
            else:
                track_tab[track] = 1
            
    return (blk_total, blk_hdr_ok, blk_dat_ok, track_tab, track_gap, short_list, long_list, long_sectors)

def recover_lost_sync_blocks(bits,long_sectors):
    blk_list = []
    for b in long_sectors:
        bit_gap = b[2] # how many bits left after the block?
        sector  = b[5] # which sector we are on?
        bit_offset = b[10] + amiga_block_bit_len # offset after the block

        while(bit_gap >= amiga_sync_block_bit_len):
            bit_gap -= amiga_sync_block_bit_len
            
            # extract lost sync bits
            lost_sync_bits = bits[bit_offset : bit_offset + amiga_sync_bit_len]
            lost_sync = decode_bin(lost_sync_bits)

            # recover block
            bit_offset += amiga_sync_bit_len
            blk_data = bits[bit_offset : bit_offset + amiga_block_bit_len]
            blk = decode_amiga_block(blk_data, bit_gap, bit_offset)
            bit_offset += amiga_block_bit_len

            # append sync
            blk.append(lost_sync)
            blk_list.append(blk)

            # reset gap of original block
            b[2] = 0
                        
    return blk_list
    
# ---------- main --------------------------------------------------------------------

def most_votes(d):
    votes = 0
    winner = -1
    for a in d:
        b = d[a]
        if b > votes:
            votes = b
            winner = a
    return winner

def do_block_analysis(blks):
    (blk_total, blk_hdr_ok, blk_dat_ok, track_tab, track_gap, short_list, long_list, long_sectors) = analyze_blocks(blks)
    track = most_votes(track_tab)
    if verbose:
        print "  total/hdr/dat:",blk_total, blk_hdr_ok, blk_dat_ok    
        print "  track no.:    ",track,"from voting",track_tab
        print "  track gap:    ",track_gap,"bits"
        print "  short sectors:",short_list
        print "  long sectors: ",long_list
    return (long_sectors, track)

def count_map_to_string(m):
    result = ""
    for a in m:
        n = m[a]
        result += "%d x %d  " % (n,a)
    return result

def print_long_block_tab(blk_tab):
    print "sectors found"
    num_complete_sectors = 0
    for i in xrange(11):
        valid_blks = len(blk_tab[i][0])
        error_blks = len(blk_tab[i][1])
        marker = ' '
        if valid_blks > 0:
            marker = '*'
            num_complete_sectors += 1
        print " %c%02d: %d(%d)" % (marker,i,valid_blks,error_blks),    
        if (i & 3) == 3:
            print            
    if(num_complete_sectors == 11):
        print " COMPLETE TRACK"
    else:
        print " partial track"

def print_short_block_tab(blk_tab, track, input_file):
    strip=""
    found=0
    for i in xrange(11):
        valid = len(blk_tab[i][0])
        if valid:
            strip += "*"
            found += 1
        else:
            strip += "-"
    if found == 11:
        rating = "OK"
    else:
        rating = "%d sectors missing" % (11-found)   
    print "%s:  Track %03d:  %s  %s" % (input_file, track, strip, rating)

# ----- img stuff -----

def save_image(img_file, data):
    global verbose
    
    if verbose:
        print "generating sample dump image...";
    
    w = len(data)
    img = Image.new("RGB",(w,255))
    draw = ImageDraw.Draw(img)
    
    # grids:
    # cell lines
    grid_col = (0,224,0)
    draw.line( (0, four_us, w-1, four_us), grid_col)
    draw.line( (0, six_us, w-1, six_us), grid_col)
    draw.line( (0, eight_us, w-1, eight_us), grid_col)
    # border lines
    grid_col = (32,64,0)
    draw.line( (0, b0, w-1, b0), grid_col)
    draw.line( (0, b1, w-1, b1), grid_col)
    draw.line( (0, b2, w-1, b2), grid_col)
    draw.line( (0, b3, w-1, b3), grid_col)

    # show noise level for spike filtering
    grid_col = (128,64,0)
    draw.line( (0, noise_level, w-1, noise_level), grid_col)
    
    pos = 0
    data_col = (255,255,255)
    weak_col = (128,128,255)
    for a in data:
        t = ord(a)
        
        # index marker
        if t == marker_index:
            draw.line( (pos, 0, pos, 255), (255,0,255))
        # overflow marker
        elif t == marker_overflow:
            draw.line( (pos, 0, pos, 255), (0,255,255))
        # data sample
        else:
            code = time_to_cellcode_num(t)
            if code == 0:
                # weak short
                draw.line((pos,0,pos,t), weak_col)
            elif code == 1:
                # weak long   
                draw.line((pos,255,pos,t), weak_col)
            elif code == 2:
                draw.line((pos,four_us,pos,t), data_col)
            elif code == 3:
                draw.line((pos,six_us,pos,t), data_col)
            elif code == 4:
                draw.line((pos,eight_us,pos,t), data_col)
        
        pos += 1


    del draw
    
    if verbose:
        print "saving sample dump to",img_file
    img.save(img_file)

# ----- do work -----

def analyze_file(input_file):
    global do_spike_filter, do_pll_filter, show_tstat, show_bstat, show_blocks, do_raw, verbose, do_img
    
    if verbose:
        print "reading track file",input_file
    data = read_track_data(input_file)
    if verbose:
        print "  got",hex(len(data)),"/",len(data),"track sample bytes"

    ovrflow = check_overflow(data)
    if ovrflow > 0 and verbose:
        print "  found",ovrflow,"overruns"
        
    # filter data
    if do_spike_filter:
        (data,num_merged) = filter_spikes(data)
        if verbose:
            print "filtering spikes: merged",num_merged,"samples"
    if do_pll_filter:
        (data,error) = filter_pll(data)
        if verbose:
            print "filtering with pll: error=",error

    # img?
    if do_img:
        save_image(input_file + ".png",data)

    # extract index
    (data,index) = extract_index(data)
    if verbose:
        print "index ranges:   ",len(index)
        print "  pos/len:      ",index

    # stats
    if show_tstat:
        t_stats = calc_time_stats(data)
        print_time_stats(t_stats)
    if show_bstat:
        b_stats = calc_cell_stats(data)
        print_cell_stats(b_stats)

    # convert the timing to cell codes: l,h,2,3,4
    codes = all_times_to_cellcodes(data)

    # convert the cell codes to bit sequences: 2 -> 10, 3 -> 100, 4 -> 1000, ignore l,h
    bits = all_cellcodes_to_bits(codes)

    # cut by amiga block sync marker
    raw_blks = cut_by_sync(bits)
    if verbose:
        print "blk syncs found:",len(raw_blks)
        print "  bit distances:",count_map_to_string(analyze_chunk_size(raw_blks))

    # decode the amiga blocks
    blks = decode_all_amiga_blocks(bits,raw_blks)
    if verbose:
        print "decoding blocks:"
        # show blocks if enabled
        if show_blocks:
            for b in blks:
                print "    ",b

    # analyze blocks
    long_sectors, track = do_block_analysis(blks)
    
    # have a look at the too long sectors
    if len(long_sectors)>0:
        blk_map = recover_lost_sync_blocks(bits,long_sectors)
        if verbose:
            print "  recov. blks:  ",len(blk_map)
        if len(blk_map) > 0:
            if show_blocks:
                for b in blk_map:
                    print "    ",b
                
            if verbose:
                print "  adding blocks and re-analyzing..."
            for b in blk_map:
                blks.append(b)
                
            # analyze blocks
            long_sectors, track = do_block_analysis(blks)
            
    # assign blocks
    blk_tab = assign_blocks(blks)
    if verbose:
        print_long_block_tab(blk_tab)
        print
    else:
        print_short_block_tab(blk_tab, track, input_file)

    # write blocks
    if write_blk:
        output_file = input_file + ".blk"
        f=open(output_file,"wb")
        empty = "\x00" * 512
        for i in xrange(11):
            valid_blks = blk_tab[i][0]
            # write an empty block
            if len(valid_blks) == 0:
                f.write(empty)
            else:
                # take first
                blk = valid_blks[0]
                blk_offset = blk[-2] + amiga_hdr_bit_len
                data = decode_amiga_mfm_data_part(bits[blk_offset : blk_offset + amiga_data_bit_len])
                f.write(data)
        f.close()

# ----- command line -----
def usage():
    print sys.argv[0],"""<options> [file]

 --tstat         print timing statistics
 --bstat         print bitcell statistics
 --blocks        print the blocks
 --raw           decode a raw file
 
 --no-spike-filter     disable short sample merge filer

 -w              write data of blocks to <file>.blk
 -f n            select filter
 -i              save image <file>.png

 -v              be more verbose
 -h              show this help

"""

show_tstat = False
show_bstat = False
do_spike_filter = True
do_pll_filter = False
show_blocks = False
do_raw = False
verbose = False
write_blk = False
do_img = False

try:
    opts, args = getopt.getopt(sys.argv[1:], "hvwi",["tstat","bstat","no-spike-filter","pll-filter","blocks","raw"])
except getopt.GetoptError, err:
    print str(err)
    usage()
    sys.exit(2)
for o, a in opts:
    if o == '-h':
        usage()
        sys.exit(0)
    elif o == '-v':
        verbose = True
    elif o == '-w':
        write_blk = True
    elif o == '--tstat':
        show_tstat = True
    elif o == '--bstat':
        show_bstat = True
    elif o == '--no-spike-filter':
        do_spike_filter = False
    elif o == '--pll-filter':
        do_pll_filter = True
    elif o == '--blocks':
        show_blocks = True
    elif o == '--raw':
        do_raw = True
    elif o == '-i':
        do_img = True
    
# get file name arg
if len(args) < 1:
    usage()
    sys.exit(1)
for file in args:
    analyze_file(file)
