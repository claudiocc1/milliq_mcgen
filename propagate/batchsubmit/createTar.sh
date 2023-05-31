#! /bin/bash

if [ -f input.tar.xz ]; then
    rm input.tar.xz
fi
tar -hcJf input.tar.xz -C .. run_sim.py configs.py  MilliqanSim/millisim/ MilliqanSim/setup.sh 


