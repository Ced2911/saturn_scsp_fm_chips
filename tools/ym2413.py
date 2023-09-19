import math
F_SAM_YM2413 = 50000
F_REF_C4 = 261

def fnum2freq(f_number, block):
    freq = 0.0
    if f_number != 0:
        freq = int(f_number * F_SAM_YM2413 * math.pow(2.0, block - 19)) # block starts from 0 (A4's fnum is 288 and b is 3 )
    
    return freq

def freq2scsp_fns(freq):
    if (freq == 0):
        return 0
    delta = 1200 * math.log2(freq / F_REF_C4)
    
    cent = delta % 1200
    fns = math.pow(2, 10) * (math.pow(2, cent/1200)-1)
    
    return fns
    
def freq2scsp_oct(freq):
    if (freq == 0):
        return 0
    delta = 1200 * math.log2(freq / F_REF_C4)
    oct = math.floor(delta / 1200)
    
    return oct

#for bloc in range(1<<3):
#    for fnum in range(1<<9):
#        freq = fnum2freq(fnum, bloc)
#        #☺print(f"b {bloc} / f {fnum} = {freq}hz === scsp {freq2scsp_fns(freq)} / {freq2scsp_oct(freq)}")

print("uint16_t ym2413_fnum_fns_map[]={")
for fnum in range(1<<9):
    freq = fnum2freq(fnum, 4)
    print(f"{int(freq2scsp_fns(freq))}, // {freq}hz")
        
print("};")