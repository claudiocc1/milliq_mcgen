import glob
import os

#python3 run_sim.py --config MQ3 --charge mu ./output.root --save_dist 2
ntuple_tag = "05252023"
sim_tag = "v1_save2m"
config = "MQ3"
dens_mult = 1.00
save_dist = 2.0
tarfile = "input.tar.xz"
# tarfile = "input_fixrock.tar.xz"

#indir = "/hadoop/cms/store/user/bemarsh/milliqan/milliq_mcgen/muons_{0}".format(ntuple_tag)
#/net/cms26/cms26r0/hmei/milliqan/muons/05252023
indir = "/net/cms26/cms26r0/hmei/milliqan/muons/{}".format(ntuple_tag)
print (indir)

os.system("mkdir -p "+"/net/cms11/data/hmei/milliqan/tmp/condor_job_logs/milliq_mcgen_muons_{0}_{1}".format(ntuple_tag,sim_tag))
os.system("mkdir -p "+"/net/cms11/data/hmei/milliqan/tmp/condor_submit_logs/milliq_mcgen_muons_{0}_{1}".format(ntuple_tag,sim_tag))

def write_header(fid):
    fid.write("""
universe=Vanilla
when_to_transfer_output = ON_EXIT
#the actual executable to run is not transfered by its name.
#In fact, some sites may do weird things like renaming it and such.
+Owner = undefined
log=/homes/hmei/tmp/condor_submit_logs/milliq_mcgen_muons_{0}_{1}/condor_12_01_16.log
output=/homes/hmei/tmp/condor_job_logs/milliq_mcgen_muons_{0}_{1}/1e.$(Cluster).$(Process).out
error =/homes/hmei/tmp/condor_job_logs/milliq_mcgen_muons_{0}_{1}/1e.$(Cluster).$(Process).err
notification=Never

executable=wrapper_ucsb.sh
""".format(ntuple_tag,sim_tag))

for sdir in glob.glob(os.path.join(indir, "*")):
    sname = os.path.split(sdir)[1]
    print (sname)
    cfgdir = "configs/muons_{0}_{1}".format(ntuple_tag, sim_tag)
    outdir = os.path.join(indir, sname, "postsim_"+sim_tag)
    print (cfgdir)
    os.system("mkdir -p "+cfgdir)
    fout = open(os.path.join(cfgdir, "cfg_{0}.cmd".format(sname)), 'w')
    write_header(fout)
    for fin in glob.glob(os.path.join(sdir, "*.root")):
        idx = fin.split("_")[-1].split(".")[0]
        #fin = fin.replace("/hadoop/cms","root://redirector.t2.ucsd.edu/")
        fout.write("\narguments={0} {1} {2} {3} {4} {5} {6}\n".format(idx, fin, config, "mu", dens_mult, save_dist, outdir))
        fout.write("queue\n")
    fout.close()

