Takes the output of `decayMCP/runDecays` or `muons/runMuons` and propagates mCPs or muons through CMS environment,
using the [MilliqanSim](https://github.com/claudiocc1/MilliqanSim) package.
Adds branches to the tree indicating position and momentum upon intersection with the detector.

Run `. setup.sh` to download the MilliqanSim repository (if this hasn't been done yet)
and set the correct `PYTHONPATH` environment variable.

There are configurable parameters at the top of `run_sim.py` that control the location
and size of the detector, as well as various simulation parameters.

After setting up, run with `python3 run_sim.py --config <cfg_name='MQ3'> --charge <charge> <input_file>`, where `cfg_name`
is the name of the setup config ("MQ3" for default milliqan, more are defined in `configs.py`), and
`charge` is the charge of the mCPs you want to simulate or "mu" if you want to run muons as oppsed to mCP.
This will propagate all of the mCPs (or muons), add branches
to the tree, throw away events where there were no hits, and output to `output.root`.
Other optional arguments are `--density-mult <mult>`, which scales the density of all materials uniformly
by `mult` (used for deriving a material systematic), and `--save_dist <dist>`, which is the distance
in meters in front of the detector face to save the particle trajectories (we use 2 m for feeding into Geant, but
defaults to 0.1).

This was very slow, but has been sped up considerably by the integration of [numba](https://numba.pydata.org) into the python looping. It runs at close to 100 Hz now.

By default, the `run_sim.py` script is set up to only save events where at least one mCP hits a 1m x 1m square around the Milliqan detector face (i.e. `does_hit_p || does_hit_m`, if using the branch names below).  This 1m x 1m is defined at the top of
configs.py

**Coordinates:** coordinates used for the position/momentum of hit are *not* in CMS coordinates.
x and y are parallel to the Milliqan detector face (x in the phi-hat direction, or vertically when standing in gallery,
and y in the eta-hat direction, or horizontally when standing in gallery), 
and z is perpendicular (positive z points *into* the detector, in the same direction as traveling particles).
x and y are centered on the center of the detector face, while z is centered at the CMS origin
(so if the detector is 33m away from the IP, the center of the detector face is (0,0,33)).

**List of branches added to the tree** (note that `*` can be either `p` or `m`, indicating
whether the branch is for the positively or negatively charged mCP):
* `sim_q`: the charge of the simulated mCPs
* `does_hit_*`: boolean indicating whether or not mCP hits 1m x 1m square surrounding detector
* `hit_*_xyz`: a TVector3 of hit position, in coordinates described above (in meters)
* `hit_*_p4`: a TLorentzVector of momentum upon hit (also in above coordinate system), in GeV
* `hit_eff`: the fraction of events with at least 1 mCP hit (*not* the fraction of mCPs that hit)
* `hit_*_nbars`: the number of unique bars (out of 18 in the demonstrator) the mCP trajectory passes through. For most trajectories this is 0 (i.e. the mCP passes through a 1m x 1m square around detector but doesn't actually hit any bars)
* `hit_*_nlayers`: the number of unique layers the mCP trajectory passes through (0 through 3)
* `hit_*_line`: boolean indicating whether the mCP trajectory passes through 3 bars in a straight line. This is nominally how we characterize a "signal-like" event
* `hit_*_bar_idxs`: list of bar indices that the mCP hits, in the order that it passes through them
* `hit_*_bar_dists`: distance traversed within the bar for each of the bar indices above

**How to run at UCSB cluster**

* Currently cms11 at UCSB has condor installed, with about 500 cores usable
* To use `ROOT` and `numba` for the fast integration (`MilliqanSim/millisim/fast_integrate.py`) a python environment could be made using [micromamba](https://mamba.readthedocs.io/en/latest/user_guide/micromamba.html)
    - Here is an example how
    ```
    # install first
    curl micro.mamba.pm/install.sh | bash

    # set up needed packages
    micromamba create python=3.6 -n milliqan_sim root numba=0.53.1 -c conda-forge
    micromamba install scipy                                        
    micromamba activate milliqan_sim                                
    ```
    ** it is then used in `propagate/batchsubmit/wrapper_ucsb.sh`

* When submitting, go to batchsubmit dir in cms11
    - python makeMuonConfigFile_ucsb.py # make changes inside as needed
    - source submit_all.sh configs
