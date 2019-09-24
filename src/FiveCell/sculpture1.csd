<CsoundSynthesizer>
<CsOptions>
; Select audio/midi flags here according to platform
; Audio out   Audio in    No messages
-odac          ;-iadc          ;;;RT audio I/O
; For Non-realtime ouput leave only the line below:
; -o moogvcf.wav -W ;;; for file output any platform
</CsOptions>
<CsInstruments>

; Initialize the global variables.
sr = 48000
kr = 4800
ksmps = 10
nchnls = 2

; Set 0dbfs to 1
0dbfs = 1

instr 1; Modal Synthesis Instrument 

idur 	init p3
iamp    init ampdbfs(p4)

kFreqScale chnget "randFreq" ; random frequency scale value sent from application

aSoftAttack	linseg	0,	0.4,	1

; to simulate the shock between the excitator and the resonator
krand	random	1,	10	
;ashock  mpulse ampdbfs(-1), krand,	2
ashock	lfo	iamp,	2	

; felt excitator from mode.csd
;aexc1	mode	ashock,	80 * (kFreqScale + 1.0),	8
aexc1	mode	ashock,	80,	8
aexc1 = aexc1 * iamp

;aexc2	mode	ashock,	188 * (kFreqScale * 1.0),	3
aexc2	mode	ashock,	188,	3
aexc2 = aexc2 * iamp

aexc = (aexc1 + aexc2)/2

;"Contact" condition : when aexc reaches 0, the excitator looses 
;contact with the resonator, and stops "pushing it"
;aexc limit aexc,0,3*iamp 

; Wine Glass with ratios from http://www.csounds.com/manual/html/MiscModalFreq.html
ares1	mode	aexc,	220 * (kFreqScale + 1),	420 ; A3 fundamental frequency

ares2	mode	aexc,	510.4,	480

ares3	mode	aexc,	935,	500

ares4	mode	aexc,	1458.6,	520

ares5	mode	aexc,	2063.6,	540

ares = (ares1+ares2+ares3+ares4+ares5)/5

gaOut1 = aexc + ares 

kRms	rms	gaOut1
	chnset	kRms,	"rmsOut"
endin

instr 6 ; Hrtf Instrument

kPortTime linseg 0.0, 0.001, 0.05 

kAzimuthVal chnget "azimuth" 
kElevationVal chnget "elevation" 
kDistanceVal chnget "distance" 
kDist portk kDistanceVal, kPortTime ;to filter out audio artifacts due to the distance changing too quickly

aLeftSig, aRightSig  hrtfmove2	gaOut1, kAzimuthVal, kElevationVal, "hrtf-48000-left.dat", "hrtf-48000-right.dat", 4, 9.0, 48000
aLeftSig = aLeftSig / (kDist + 0.00001)
aRightSigs = aRightSig / (kDist + 0.00001)
	
aL = aLeftSig
aR = aRightSig

outs	aL,	aR
endin

instr 7 ;test tone

kamp = ampdbfs(-3) 
kcps = 440

asig 	oscil 	kamp, 	kcps 	
	outs 	asig, 	asig

endin

</CsInstruments>
<CsScore>
;p1	p2	p3	p4	p5	p6	p7	p8	p9	p10	p11	p12	p13	p14	p15	p16	p17	p18	p19	p20	p21	p22	p23	p24

i1	3	180	-2		

i6	2	180	

;i7	0	240

</CsScore>
<
