This directory contains scripts necessary to generate muons from four distinct production modes:

	i) `Heavy Flavour QCD`
	ii) `W Decays`
	iii) `Drell-Yan`
	iv) `Decays in flight`

The instruction to run the muon generation are:
1. Run `make` in the current directory ( or `make clean` followed by `make` if an outdated compiled version of the code exists)

2. Usage:
```
./runMuons -d production_mode -o outfile [-n n_events=1000]  \
 					[-N n_events_total=n_events] [-e evtnum_offset=0]
```

`outfile` is the name of the ROOT file to output to.

`n_events` is the number of events to generate.

`n_events_total` is a constant copied into the tree directly, and is meant to represent the total number of events generated in a given sample
(for the case where one sample is split over many files). This makes it easier to compute the per-event weight when looping over events later.

`evtnum_offset` is the value at which to start numbering events (so if you're making multiple files, event numbers don't overlap).

**Note:** the milliQan demonstrator eta/phi location is hardcoded into runDecays.cc. If this changes, you need to edit this.


### File format
Output root tree has the following branches:
* `event`: integer event number, starting from 0
* `n_events_total`: total number of events in the sample. Defaults to the number of events in the given file, but can be overridden
in the case that the sample is split over many files (see above)
* `decay_mode`: copy of the `decay_mode` argument to the program, defined above
* `p4_p`: four-momentum of positively-charged mCP (in GeV)
* `p4_m`: four-momentum of negatively-charged mCP (in GeV)


### Grid submission

I've now added scripts to `localbatch` to do all of the above with a single condor submission, allowing for larger samples and far less babysiting.
To run on the grid, create a input tarball first with: `./createTar.sh`, then run `python make_config.py`.
You may want to configure some output directories and tags / njobs / etc. inside `make_config.py`. After the production completed successfully, if you wish, you can cleanup the logs with the `cleanup.py` script.

* for things to run at UCSB cluster, one could use `make_config_ucsb.py` and `wrapper_ucsb.sh`
