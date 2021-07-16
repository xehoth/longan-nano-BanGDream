from essentia.standard import *
import sys
import random
import os

if len(sys.argv) <= 1:
    print("please input music file name")
    sys.exit(0)

name = sys.argv[1][:-5]
FPS = 30

with open(f'{name}l.txt', 'w') as leftF, open(f'{name}r.txt', 'w') as rightF:
    audio = MonoLoader(filename=sys.argv[1])()
    od1 = OnsetDetection(method='hfc')
    od2 = OnsetDetection(method='complex')
    w = Windowing(type = 'hann')
    fft = FFT() 
    c2p = CartesianToPolar()
    pool = essentia.Pool()

    for frame in FrameGenerator(audio, frameSize = 1024, hopSize = 512):
        mag, phase, = c2p(fft(w(frame)))
        pool.add('features.hfc', od1(mag, phase))
        pool.add('features.complex', od2(mag, phase))

    onsets = Onsets()

    onsets_hfc = onsets(essentia.array([ pool['features.hfc'] ]), [ 1 ])

    onsets_complex = onsets(essentia.array([ pool['features.complex'] ]), [ 1 ])
    left = []
    right = []

    for x in onsets_complex:
        val = (0, int(x*FPS))
        r = random.randint(0, 2)
        if r==0:
            left.append(val)
        elif r== 1:
            right.append(val)
        else:
            left.append(val)
            right.append(val)
    
    for i in left:
        print(i[0], i[1], file=leftF)
    for i in right:
        print(i[0], i[1], file=rightF)