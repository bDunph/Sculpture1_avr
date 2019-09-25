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

;*************************************************************************************
instr 1; Modal Synthesis Percussive Instrument 
;*************************************************************************************
idur 	init p3
iamp    init ampdbfs(p4)

kFreqScale chnget "randFreq" ; random frequency scale value sent from application

; to simulate the shock between the excitator and the resonator
krand	random	1,	10	
ashock  mpulse ampdbfs(-1), krand,	2

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
aexc limit	aexc,	0,	3*iamp 

; Wine Glass with ratios from http://www.csounds.com/manual/html/MiscModalFreq.html
;ares1	mode	aexc,	220 * (kFreqScale + 1),	420 ; A3 fundamental frequency
ares1	mode	aexc,	220,	420 ; A3 fundamental frequency

ares2	mode	aexc,	510.4,	480

ares3	mode	aexc,	935,	500

ares4	mode	aexc,	1458.6,	520

ares5	mode	aexc,	2063.6,	540

ares = (ares1+ares2+ares3+ares4+ares5)/5

gaOut1 = aexc + ares 

kRms	rms	gaOut1
	chnset	kRms,	"rmsOut"
endin

;*************************************************************************************
instr 2	; Physical Bowed String Instrument
;*************************************************************************************
kpres = 4
krat = 0.046
kvibf = 6.12723

kvib	linseg	0,	0.5,	0,	1,	1,	2.5,	1
kvamp = kvib * 0.01

asig	wgbow	0.7,	55,	kpres,	krat,	kvibf,	kvamp

	outs	asig,	asig

endin
			
;*************************************************************************************
instr 3 ; Physical Bowed Bar Instrument
;*************************************************************************************

kEnv	adsr	0.45,	0.08,	0.9,	0.6

kp = 1.9 

asig	wgbowedbar	ampdbfs(-1),	133,	0.7,	kp,	0.969
asig = asig * kEnv

     outs asig, asig

endin

;*************************************************************************************
instr 4 ; Waveguide + Mode Instrument - Noise Filter
;*************************************************************************************

;envelope
kEnv	adsr	0.3,	0.1,	0.8,	0.6

;noise
kbeta	line	-0.9999,	p3,	0.9999
asig	noise	ampdbfs(-3),	0.3999	
asig = asig * kEnv

;waveguide model1
kfreq		init	200
kcutoff		init	300
kfeedback	init	0.2

awg1	wguide1	asig,	kfreq,	kcutoff,	kfeedback

;waveguide model2
kfreq2		linseg	800,	p3/2,	100,	p3/2,	300
kcutoff2	linseg	500,	p3/2,	700,	p3/2,	200
kfeedback2	linseg	0.5,	p3/2,	0.8,	p3/2,	0.1

awg2	wguide1	awg1,	kfreq2,	kcutoff2,	kfeedback2	

;tone filter
khp	linseg	700,	p3/4,	350,	p3/4,	400,	p3/4,	300,	p3/4,	200

aToneOut	tone	awg2,	khp

;mode bank
; felt excitator from mode.csd
iamp	init	ampdbfs(-3)

kModeFreq	linseg	80,	p3/2,	99,	p3/2,	89
kQFactor	linseg	8,	p3/2,	21,	p3/2,	18

aexc1	mode	aToneOut,	kModeFreq,	kQFactor	
aexc1 = aexc1 * iamp 

kModeFreq2	linseg	188,	p3/2,	159,	p3/2,	201
kQFactor2	linseg	3,	p3/2,	15,	p3/2,	5

aexc2	mode	aToneOut,	kModeFreq2,	kQFactor2
aexc2 = aexc2 * iamp 

aexc = (aexc1 + aexc2)/2

	outs	aexc,	aexc

endin

;**************************************************************************************		
instr 5 ; Beaten Plate 
;**************************************************************************************		

kFreq	linseg	50,	p3,	20
kPhase	phasor	5
kAmp = 0.4 * kPhase

aSig		lfo	kAmp,	kFreq,	1
aFreq1		linseg	900,	p3/2,	870,	p3/2,	1100
aFreq2		linseg	500,	p3/3,	340,	p3/3,	624,	p3/3,	300
kCutOff1 = 3000
kCutOff2 = 1500
kFeedback1 = 0.35
kFeedback2 = 0.15

aWGOut	wguide2	aSig,	aFreq1,	aFreq2,	kCutOff1,	kCutOff2,	kFeedback1,	kFeedback2
aWGOut	dcblock2	aWGOut

	outs	aWGOut,	aWGOut

endin

;**************************************************************************************		
instr 6 ; Hrtf Instrument
;**************************************************************************************
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

;********************************************************************
instr 7 ;test tone
;********************************************************************

kamp = ampdbfs(-3) 
kcps = 440

asig 	oscil 	kamp, 	kcps 	
	outs 	asig, 	asig

endin

</CsInstruments>
<CsScore>
;p1	p2	p3	p4	p5	p6	p7	p8	p9	p10	p11	p12	p13	p14	p15	p16	p17	p18	p19	p20	p21	p22	p23	p24

;i1	2	10	-2		

;i2	2	2
;i2	+	2
;i2	+	2	

i3	2	10
i3	+	5
i3	+	2

;i4	30	10
;i4	+	5
;i4	+	2
;
;i5	56	20
;i5	+	10
;i5	+	5

;i6	2	180	

;i7	0	240

</CsScore>
<
