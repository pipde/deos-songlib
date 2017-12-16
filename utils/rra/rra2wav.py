import sys

# change PROGRAM_NAME and PROGRAM_VERSION appropriately

PROGRAM_NAME = "rra2wav (simplified python3 version)"
PROGRAM_VERSION = "0.01"

def main():
    if (len(sys.argv) == 1):
        s = sys.stdin
    elif (len(sys.argv) == 2):
        s = open(sys.argv[1],"r")
    else:
        print("usage: python3 rra2wav.py RRA_FILENAME > WAV_FILENAME")
        print("   or: python3 rra2wav.py < RRA_FILENAME > WAV_FILENAME")
        print
        sys.exit(1)

    filter(s)
    s.close()
    return 0

def filter(s):

    sr,ch,bps,samples,amp = readRRAHeader(s)

    writeWaveHeader(sr,ch,bps,samples)

    for i in range(0,samples,1):
        for j in range(0,ch,1):
            printByteInt(amp,bps//8)
            amp = readint(s)

def readRRAHeader(s):

    # default RRA values
    sampleRate = 44100
    bps = 16
    channels = 1
    samples = 10 * sampleRate # default is 10 seconds worth of samples

    attribute = readtoken(s)

    if (attribute == ""):
        print("not an RRA file: first attribute should be RRAUDIO")
        sys.exit(1)

    # check if raw data file -- if so, just return
    if (attribute[0].isdigit()):
        return sampleRate,channels,bps,samples,int(attribute) #attribute is a sample

    if (attribute != "RRAUDIO"):
        print("not an RRA file: first attribute is",attribute,"(should be RRAUDIO)")
        sys.exit(1)

    attribute = readtoken(s)
    while (attribute != "%%"):

        value = readtoken(s)

        if (attribute == "samples:"):
            samples = int(value)
        elif (attribute == "sampleRate:"):
            sampleRate = int(value)
        elif (attribute == "bitsPerSample:"):
            bps = int(value)
        elif (attribute == "channels:"):
            channels = int(value)
        #else:
            #print("ignoring RRA attribute:",attribute,file=sys.stderr)

        attribute = readtoken(s)

    amp = readint(s)
    return sampleRate,channels,bps,samples,amp #attribute is a sample

def writeWaveHeader(rate,channels,bps,samples):
    sys.stdout.buffer.write(b"RIFF")
    printByteInt(36 + samples * channels * (bps // 8),4)
    sys.stdout.buffer.write(b"WAVE")
    sys.stdout.buffer.write(b"fmt ")
    printByteInt(16,4) #PCM subchunk
    printByteInt(1,2) #PCM (no compression)
    printByteInt(channels,2)  #1 for mono, 2 for stereo
    printByteInt(rate,4) #sample rate
    printByteInt(rate * channels * (bps // 8),4) #byte rate
    printByteInt(channels * (bps // 8),2) #block align
    printByteInt(bps,2) #bits per sample
    sys.stdout.buffer.write(b"data")
    printByteInt(samples * channels * (bps // 8),4) #subchunk 2 size

def printByteInt(n,size):

    b = [None]*size
    for i in range(0,size):
       b[i] = n % 256
       n = n // 256

    b = bytes(b)
    #print("b is",b,file=sys.stderr)

    sys.stdout.buffer.write(b)
       
tokens = []
tokenIndex = 0

def readtoken(fp):
    global tokens,tokenIndex
    if (tokenIndex == len(tokens)):
        tokens=fp.readline().split()
        if (len(tokens) == 0): return ""
        tokenIndex = 0
    tokenIndex += 1
    return tokens[tokenIndex-1]

def readint(fp):
    t = readtoken(fp)
    if (t != ""):
        t = int(t)
    return t

main()
