import os
import sys
import numpy as np

def Usage():
	print 'RankDistanceModelsByNOE.py TPLfreeNOEfile seqFile'
	

def LoadFASTAFile(seqFile):

        ##load in the seq file
        seqfh = open(seqFile, 'r')
        content = [ line.strip() for line in list(seqfh) ]
        seqfh.close()
        content2 = [ c for c in content if not c.startswith('>') and not c.startswith('#') ]
        sequence = ''.join(content2)

        return sequence

def LoadNOEFile(NOEfile):
	fh = open(NOEfile, 'r')
	content = [ line.strip() for line in list(fh) ]
	fh.close()

	## parse content
	assert len(content) % 6 ==0

	#print len(content)

	data = []
	for i in range(0, len(content), 6):
		modelFile = content[i]
		modelFlag = 'EC52C'

		if not modelFlag in modelFile:
			modelFlag = 'EC25C'

		## read the 5 lines of NOE scores
		NOEs = []
		for j in range(1, 6):
			line = content[i+j]
			fields = line.split()
			assert len(fields) == 3
			avgNOE = np.float32(fields[2])
			NOEs.append(avgNOE)

		if NOEs[-1]*1./NOEs[0]>1.1:
			print 'WARNING: two top models generated by the same settings have NOEs score of very different'
			print 'ModelDir=', modelFile 

		noe = np.average(NOEs)

		data.append( [noe, modelFile] )

	return data



if len(sys.argv)<3:
	Usage()
	exit(-1)

NOEfile=sys.argv[1]
seqFile=sys.argv[2]

proteinName = os.path.basename(NOEfile).split('.')[0]

sequence=LoadFASTAFile(seqFile)
seqLen = len(sequence)

NOEs = LoadNOEFile(NOEfile)

## add seqLen information, 0 is the alignment length
data = [ [seqLen, 0 ] + noe for noe in NOEs ]

data.sort(key=lambda x: x[2])

for d in data:

	dstr = [proteinName] + [ "%.3f" % d0 for d0 in d[:-1] ] + [d[-1]]
	print ' '.join(dstr)
