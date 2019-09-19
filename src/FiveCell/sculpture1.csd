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

; to simulate the shock between the excitator and the resonator
krand	random	1,	10	
ashock  mpulse ampdbfs(-1), krand,	2
;ashock	lfo	iamp,	2

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
aexc limit aexc,0,3*iamp 

; Wine Glass with ratios from http://www.csounds.com/manual/html/MiscModalFreq.html
ares1	mode	aexc,	220 * (kFreqScale + 1),	420 ; A3 fundamental frequency

ares2	mode	aexc,	510.4,	480

ares3	mode	aexc,	935,	500

ares4	mode	aexc,	1458.6,	520

ares5	mode	aexc,	2063.6,	540

ares = (ares1+ares2+ares3+ares4+ares5)/5

;envelope to shape output
;iatt = 0.8
;idec = 0.5
;isus = 0.01
;irel = 0.7
;
;aenv	adsr	iatt,	idec,	isus,	irel

gaOut1 = aexc + ares 

kRms	rms	gaOut1
	chnset	kRms,	"rmsOut"
endin

instr 6 ; Hrtf Instrument

S_AzimuthVals[] init 5
S_ElevationVals[] init 5
S_DistanceVals[] init 5

iCount = 0
loop:
	S_VertNumber sprintf "%i", iCount

	S_AzimuthChannel strcpy "azimuth"	
	S_ChannelNameAz strcat S_AzimuthChannel, S_VertNumber
	S_AzimuthVals[iCount] sprintf "%s", S_ChannelNameAz

	S_ElevationChannel strcpy "elevation"
	S_ChannelNameEl strcat S_ElevationChannel, S_VertNumber
	S_ElevationVals[iCount] sprintf "%s", S_ChannelNameEl

	S_DistanceChannel strcpy "distance"
	S_ChannelNameDist strcat S_DistanceChannel, S_VertNumber
	S_DistanceVals[iCount] sprintf "%s", S_ChannelNameDist

	loop_lt iCount, 1, 5, loop

kAzimuthVals[] init 5
kElevationVals[] init 5
kDistanceVals[] init 5

kPortTime linseg 0.0, 0.001, 0.05 

kAzimuthVals[0] chnget S_AzimuthVals[0] 
kElevationVals[0] chnget S_ElevationVals[0] 
kDistanceVals[0] chnget S_DistanceVals[0] 
kDist0 portk kDistanceVals[0], kPortTime ;to filter out audio artifacts due to the distance changing too quickly

;kAzimuthVals[1] chnget S_AzimuthVals[1] 
;kElevationVals[1] chnget S_ElevationVals[1] 
;kDistanceVals[1] chnget S_DistanceVals[1] 
;kDist1 portk kDistanceVals[1], kPortTime ;to filter out audio artifacts due to the distance changing too quickly
;
;kAzimuthVals[2] chnget S_AzimuthVals[2] 
;kElevationVals[2] chnget S_ElevationVals[2] 
;kDistanceVals[2] chnget S_DistanceVals[2] 
;kDist2 portk kDistanceVals[2], kPortTime ;to filter out audio artifacts due to the distance changing too quickly
;
;kAzimuthVals[3] chnget S_AzimuthVals[3] 
;kElevationVals[3] chnget S_ElevationVals[3] 
;kDistanceVals[3] chnget S_DistanceVals[3] 
;kDist3 portk kDistanceVals[3], kPortTime ;to filter out audio artifacts due to the distance changing too quickly
;
;kAzimuthVals[4] chnget S_AzimuthVals[4] 
;kElevationVals[4] chnget S_ElevationVals[4] 
;kDistanceVals[4] chnget S_DistanceVals[4] 
;kDist4 portk kDistanceVals[4], kPortTime ;to filter out audio artifacts due to the distance changing too quickly

aRightSigs[] init 5
aLeftSigs[] init 5 

aLeftSigs[0], aRightSigs[0]  hrtfmove2	gaOut1, kAzimuthVals[0], kElevationVals[0], "hrtf-48000-left.dat", "hrtf-48000-right.dat", 4, 9.0, 48000
aLeftSigs[0] = aLeftSigs[0] / (kDist0 + 0.00001)
aRightSigs[0] = aRightSigs[0] / (kDist0 + 0.00001)
	
;aLeftSigs[1], aRightSigs[1]  hrtfmove2	gaOut2, kAzimuthVals[1], kElevationVals[1], "hrtf-48000-left.dat", "hrtf-48000-right.dat", 4, 9.0, 48000
;aLeftSigs[1] = aLeftSigs[1] / (kDist1 + 0.00001)
;aRightSigs[1] = aRightSigs[1] / (kDist1 + 0.00001)
;
;aLeftSigs[2], aRightSigs[2]  hrtfmove2	gaOut3, kAzimuthVals[2], kElevationVals[2], "hrtf-48000-left.dat", "hrtf-48000-right.dat", 4, 9.0, 48000
;aLeftSigs[2] = aLeftSigs[2] / (kDist2 + 0.00001)
;aRightSigs[2] = aRightSigs[2] / (kDist2 + 0.00001)
;
;aLeftSigs[3], aRightSigs[3]  hrtfmove2	gaOut4, kAzimuthVals[3], kElevationVals[3], "hrtf-48000-left.dat", "hrtf-48000-right.dat", 4, 9.0, 48000
;aLeftSigs[3] = aLeftSigs[3] / (kDist3 + 0.00001)
;aRightSigs[3] = aRightSigs[3] / (kDist3 + 0.00001)
;
;aLeftSigs[4], aRightSigs[4]  hrtfmove2	gaOut5, kAzimuthVals[4], kElevationVals[4], "hrtf-48000-left.dat", "hrtf-48000-right.dat", 4, 9.0, 48000
;aLeftSigs[4] = aLeftSigs[4] / (kDist4 + 0.00001)
;aRightSigs[4] = aRightSigs[4] / (kDist4 + 0.00001)

;aL = (aLeftSigs[0] + aLeftSigs[1] + aLeftSigs[2] + aLeftSigs[3] + aLeftSigs[4]) / 5
;aR = (aRightSigs[0] + aRightSigs[1] + aRightSigs[2] + aRightSigs[3] + aRightSigs[4]) / 5

aL = aLeftSigs[0]
aR = aRightSigs[0]

;aLimL	limit	aL,	ampdbfs(-96),	ampdbfs(0)
;aLimR	limit	aR,	ampdbfs(-96),	ampdbfs(0)

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
