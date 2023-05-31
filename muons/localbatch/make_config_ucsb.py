import os

'''
tag = "Monash2013"
nevts_per_job = 100000
njobs = 500
offset = 0
'''

tag = "05252023"
nevts_per_job = 10000
njobs = 100
offset = 0


#cp myoutput.txt /net/cms26/cms26r0/hmei/ 
base_dir="/net/cms26/cms26r0/hmei/milliqan/muons/"

os.system("mkdir -p %s"%(base_dir))

names = {"qcd":1,"w":2,"dy":3,"qcd_nonbc":4}
#names = {"w":2,"dy":3}

for process in names.keys() :
    mode = int(names[process])
    outdir = os.path.join(base_dir,tag,process)
    os.system("mkdir -p %s"%(outdir))
    os.system("mkdir -p %s/logs"%(outdir))

    fout = open("config.cmd",'w')
    fout.write("""
    universe=Vanilla
    when_to_transfer_output = ON_EXIT

    +Owner = undefined

    log=%s/logs/submit_$(Cluster)_$(Process).log
    output=%s/logs/1e.$(Cluster).$(Process).out
    error =%s/logs/1e.$(Cluster).$(Process).err

    +project_Name = "cmssurfandturf"

    executable=wrapper_ucsb.sh
    """%(outdir,outdir,outdir)
    )

    run = False
    for ijob in range(njobs):
        if not os.path.isfile("%s/output_%s.root"%(outdir,str(ijob+1+offset))):
          fout.write("arguments={0} {1} {2} {3} {4}\nqueue\n\n".format(ijob+1+offset, mode, nevts_per_job, nevts_per_job*njobs, outdir))
          if not run:
            run = True

    fout.close()
    if run:
      print("Now processing: " + process + "." )
      os.system("condor_submit config.cmd")
