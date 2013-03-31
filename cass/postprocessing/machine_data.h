//Copyright (C) 2010 Lutz Foucar

/**
 * @file machine_data.h file contains declaration of postprocessors that
 *                      extract information from the beamline and epics data.
 *
 * @author Lutz Foucar
 */

#ifndef _MACHINE_DATA_H_
#define _MACHINE_DATA_H_

#include <string>

#include "postprocessor.h"
#include "backend.h"
#include "cass_event.h"

namespace cass
{
  //forward declaration
  class Histogram0DFloat;



  /** retrieval of beamline data.
   *
   * This postprocessor will retrieve the requested Beamline Data from
   * the cass event.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{VariableName}
   *           The name of the beamline data variable you are interested in.
   *           Default is "". Available values are:
   *           - FEE Gas Detector values
   *             - f_11_ENRC
   *             - f_12_ENRC
   *             - f_21_ENRC
   *             - f_22_ENRC
   *           - E-Beam values
   *             - EbeamCharge
   *             - EbeamL3Energy
   *             - EbeamLTUAngX
   *             - EbeamLTUAngY
   *             - EbeamLTUPosX
   *             - EbeamLTUPosY
   *             - EbeamPkCurrBC2
   *           - Phase Cavity values
   *             - Charge1
   *             - Charge2
   *             - FitTime1
   *             - FitTime2
   *           - Ipimb values
   *             - %DetectorName%_Channel0
   *             - %DetectorName%_Channel1
   *             - %DetectorName%_Channel2
   *             - %DetectorName%_Channel3
   *           - IpmFex values
   *             - %DetectorName%_CorrectChannel0
   *             - %DetectorName%_CorrectChannel1
   *             - %DetectorName%_CorrectChannel2
   *             - %DetectorName%_CorrectChannel3
   *             - %DetectorName%_sum
   *             - %DetectorName%_xPos
   *             - %DetectorName%_yPos
   *
   * @author Lutz Foucar
   */
  class pp120 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp120(PostProcessors& hist, const PostProcessors::key_t&);

    /** copy data from CASS event to histogram storage */
    virtual void process(const CASSEvent&);

    /** load the settings from cass.ini */
    virtual void loadSettings(size_t);

  protected:
    /** name of the variable in the beamline data */
    std::string _varname;
  };







  /** check whether event contains eventcode
   *
   * This postprocessor will check whether an eventcode is present in the event.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{EventCode}
   *           The EventCode to check for. Default is 0
   *
   * @author Lutz Foucar
   */
  class pp121 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp121(PostProcessors& hist, const PostProcessors::key_t&);

    /** copy data from CASS event to histogram storage */
    virtual void process(const CASSEvent&);

    /** load the settings from cass.ini */
    virtual void loadSettings(size_t);

  protected:
    /** name of the variable in the beamline data */
   size_t _eventcode;
  };










  /** retrieve the eventId from event
   *
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @author Lutz Foucar
   */
  class pp122 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp122(PostProcessors& hist, const PostProcessors::key_t&);

    /** copy data from CASS event to histogram storage */
    virtual void process(const CASSEvent&);

    /** load the settings from cass.ini */
    virtual void loadSettings(size_t);
  };











  /** retrieval of Epics data.
   *
   * This postprocessor will retrieve the requested epics data from the cass-event.
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @cassttng PostProcessor/\%name\%/{VariableName}
   *           The name of the epics data variable you are interested in.
   *           Default is "".  If the EPICS variable is not part of the standart
   *           list, but contained in an additional list, you have to prepend
   *           the additional lists name to the epics variable. For a complete
   *           list of available variables, please look into the casslog, when
   *           the logging level is set to INFO
   *
   * @author Lutz Foucar
   */
  class pp130 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp130(PostProcessors& hist, const PostProcessors::key_t&);

    /** copy data from CASS event to histogram storage */
    virtual void process(const CASSEvent&);

    /** load the settings from cass.ini */
    virtual void loadSettings(size_t);

  protected:
    /** name of the variable in the beamline data */
    std::string _varname;
  };










  /** retrieve photonenergy.
   *
   * This postprocessor will calculate the photonenergy from the BLD. Here is an
   * email corrospondence from Andy Acquila and Anton Barty explaining where the
   * calculation comes from:
   *
@verbatim
Hi Benedikt,

I wish I had a paper about the wakefield correction, however I can't find it
However below is an email from James (Jim) Welch to Mark Messerschmidt.
It contains matlab code that we copied for the correction.  Also attached is a
plot from Anton of the effect of the correction as compared to the undulator
equation.

Please let me know if you have any questions.

Cheers,
Andy

Marc,

The formula I use calculates the resonant photon energy based on an estimate of
the electron beam energy at the first in-line undulator segment, and the K of
that segment.

The energy estimate for the first in-line segment starts with the DL2 bend
energy, and on a shot by shot basis, adds a correction based on the DL2 bpms for
the incoming energy, a correction for the wakefield energy loss which depends on
the measured peak bunch current (averaged over 10 shots), and a correction for
the spontaneous energy loss due to emission from the undulator segments.

The matlab code is below. We still have some problems coming up with good values
for wakeloss, especially if beam conditions are unusual. Also there may be a
slight shift between the resonant photon energy and the FEL peak of the
spectrum. Another factor that is uncertain is what exact value to use for the K
in the resonance formula. Any suggestions would be appreciated.

       - Jim


function photonEnergyeV = photonEnergyeV(DL2energyGeV,
peakCurrent, charge,K,xltu250, xltu450, display_output)
%
% photonEnergyeV = photonEnergyeV(DL2energyGeV,peakCurrent,charge,K,xltu25, xltu450, display_output)
%
% Return the resonant photon energy eV for the fundamental based on the peak
% current, DL2 beam energy, and K of first active segment, beam position in
% the two dogleg bpms.
%
% If no input arguments are present it will get the input from the machine

% If no input, parameters are set actual machine parameters
if nargin==0
  display_output = 0;  % set to 1 to do displays

  % Get the present peak current
  for q=1:10 % average over several reads
      temp(q) = lcaGet('BLEN:LI24:886:BIMAX');
  end
  peakCurrent=  mean(temp);

  % Get present charge in pC
  charge = 1000*lcaGet('FBCK:BCI0:1:CHRGSP');

  % Get present beam energy in DL2
  npts = 100;
  [xltu250, xltu450 ] = bpmDoglegRead(npts);
  if any([xltu250, xltu450]==0); % if bpms not reading, assume zero
      xltu250=0; xltu450=0;
  end
  DL2energyGeV = energyCorrectDL2(xltu250, xltu450, display_output);
  if display_output
      display(['DL2 energy ' num2str(DL2energyGeV) 'GeV' ]);
  end
end

% Get the present taper configuration
taper = segmentTranslate;

% Get total energy loss in each segment. Extracted segments have only wake
% loss
activeSegments = find(taper < 11); % these are active
energyLossPerSegment(1:33) = wakefield(peakCurrent, charge, display_output); % MeV loss per segment
energyLossPerSegment(activeSegments) = ...% add wakefield to active segments
  energyLossPerSegment(activeSegments) + SRloss( DL2energyGeV, display_output);

% Calculate energy profile [GeV]
energyProfile(1:33) = DL2energyGeV - LTUwakeLoss(peakCurrent,charge, display_output)/1000;
for q=1:33 % energyProfile represents average electron energy in each segment.
  energyProfile(q) = energyProfile(q)...
      - 0.001*sum(energyLossPerSegment(1:q)) - 0.0005*energyLossPerSegment(q);
end

% Calculate the resonant photon energy of the first active segment)
for q=1:33
  pvKACT{q,1} = sprintf('USEG:UND1:%d50:KACT',q);
end
if nargin==0
  Kprofile = lcaGetSmart(pvKACT);
else
  Kprofile = K*ones(33,1); % pretend they are all the same if K is supplied
end
photonEnergyeV = 8265.9 * ( energyProfile( activeSegments(1) ) / 13.64 )^2 *...
   (1 + 3.5^2/2) /(1 + Kprofile( activeSegments(1) )^2 / 2 ) ;
if display_output
  display(['1st harmonic photon energy: ', num2str(photonEnergyeV,'%5.0f') ' eV']);
end


function wakeLossPerSegment = wakefield(peakCurrent, charge, display_output)
% return the wake field induced energy loss per electron per segment
% Assume proportion to peak current.
% wakeLossPerSegment = 0.15 * peakCurrent/500; % MeV/segment, from Juhao 8/09
% if abs(peakCurrent) > 4000
%     wakeLossPerSegment = 0.15 * 300/500; % MeV/segment, adjusted to match 20 pC meas 10/6/09
% end

% Use Nuhn calculation for undulator
segmentLength = mean(diff(segmentCenters));
compressState = charge>25; % assume undercompressed for more than 25 pC, else overcompressed
wakeLossPerSegment = segmentLength * 0.001 *...
  util_UndulatorWakeAmplitude(abs(peakCurrent)/1000, charge, compressState);
if display_output
  display([ 'Wake loss/segment ' num2str(-wakeLossPerSegment,2) ' MeV']);
end

function LTUwakeLoss = LTUwakeLoss(peakCurrent, charge, display_output)
% Add LTU loss per Novohatsky
LTUfactor = 7.8; % MeV loss for 20 pC 0.5 micron rms bunch length
peakCurrentRef = 20e-12 * 3e8/0.5e-6/sqrt(2*pi); % Amps of peak current for LTUfactor
LTUwakeLoss = LTUfactor * (peakCurrent/peakCurrentRef);% scale with peak current^2/Q
if display_output
  display(['LTU wake loss ', num2str(LTUwakeLoss,2) ' MeV']);
end

function SRlossPerSegment = SRloss(electronEnergy, display_output)
% returns energy loss per segment [MeV] from spontaneous radiation in MeV
SRlossPerSegment = 0.63 * (electronEnergy/13.64)^2;
if display_output
  display(['SR loss/segment ', num2str(SRlossPerSegment,2) ' MeV']);
end

% This function is not used but could be added
function DL2energyGeV = energyCorrectDL2(xltu250, xltu450, display_output)
% correctedEnergy = energyCorrectDL2(xltu250, xltu450)
%
% returns the "corrected" = measured energies for a set of BSA bpm readings based on
% DL2 bpm readings and the DL2 magnet strengths. Use an average of the last
% npts stored in BSA buffers

bendEnergyGeV = lcaGetSmart('BEND:DMP1:400:BDES'); %use dump bend power supply
etax = .125 ; % [m] design value for dispersion at bpms in dogleg
DL2energyGeV = bendEnergyGeV + bendEnergyGeV*0.001*(xltu250(1,:)...
  - xltu450(1,:))/(2*etax);
if display_output
  display(['DL2 bend energy ' num2str(bendEnergyGeV) 'GeV' ]);
end

function [xltu250, xltu450 ] = bpmDoglegRead(npts, display_output)
% Return the averaged dogleg2 bpm x readings from buffered data
pvBPM = {'BPMS:LTU1:250:XHSTBR'; 'BPMS:LTU1:450:XHSTBR'};
bpms = lcaGet(pvBPM, npts);
xltu250 = mean(bpms(1,:));
xltu450 = mean(bpms(2,:));


On Aug 10, 2011, at 5:52 AM, Anton Barty wrote:

> Hi Benedikt
>
> At a conference right now, so no web access for papers.
> I'll cc' to Andy who may be able to dig up a paper online.
>
>
> The formulae came from LCLS machine physics, possibly via Marc Messerschmidt.
>
> To first order, it's the standard undulator equation - which requires such
> stuff as undulator K-factors (known from the undulator design and calibrations).
> This is the energy at which SASE lasing initiates.  The formulae basically
> relate electron beam energy to
>
> You can find a summary of formulae here (in B.2 - undulator radiation)
> http://xdb.lbl.gov/Section2/Sec_2-1.html
>
> The other terms are corrections to the electron energy due to wakefield losses
> and the like and make for relatively small corrections (<1%)  to the undulator
> energy.
>
@endverbatim
   *
   * @see PostprocessorBackend for a list of all commonly available cass.ini
   *      settings.
   *
   * @author Lutz Foucar
   */
  class pp230 : public PostprocessorBackend
  {
  public:
    /** constructor */
    pp230(PostProcessors& hist, const PostProcessors::key_t&);

    /** calc the photonenergy from the bld */
    virtual void process(const CASSEvent&);

    /** load the settings from cass.ini */
    virtual void loadSettings(size_t);
  };
}

#endif
