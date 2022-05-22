#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <string.h>
#include <vector>
#include <omp.h>
#include <getopt.h>
#include "cdhit_seqdb.h"
using namespace std;


//-------- utility ------//
void getBaseName(string &in,string &out,char slash,char dot)
{
	int i,j;
	int len=(int)in.length();
	for(i=len-1;i>=0;i--)
	{
		if(in[i]==slash)break;
	}
	i++;
	for(j=len-1;j>=0;j--)
	{
		if(in[j]==dot)break;
	}
	if(j==-1)j=len;
	out=in.substr(i,j-i);
}
void getRootName(string &in,string &out,char slash)
{
	int i;
	int len=(int)in.length();
	for(i=len-1;i>=0;i--)
	{
		if(in[i]==slash)break;
	}
	if(i<=0)out=".";
	else out=in.substr(0,i);
}

//=================== upper and lower case ====================//
//----------upper_case-----------//
void toUpperCase(char *buffer)
{
	for(int i=0;i<(int)strlen(buffer);i++)
	if(buffer[i]>=97 && buffer[i]<=122) buffer[i]-=32;
}
void toUpperCase(string &buffer)
{
	for(int i=0;i<(int)buffer.length();i++)
	if(buffer[i]>=97 && buffer[i]<=122) buffer[i]-=32;
}
//----------lower_case-----------//
void toLowerCase(char *buffer)
{
	for(int i=0;i<(int)strlen(buffer);i++)
	if(buffer[i]>=65 && buffer[i]<=90) buffer[i]+=32;
}
void toLowerCase(string &buffer)
{
	for(int i=0;i<(int)buffer.length();i++)
	if(buffer[i]>=65 && buffer[i]<=90) buffer[i]+=32;
}

//----- get upper case -----//
int getUpperCase(char *buffer)
{
	int count=0;
	for(int i=0;i<(int)strlen(buffer);i++)
	if(buffer[i]>=65 && buffer[i]<=90) count++;
	return count;
}
int getUpperCase(string &buffer)
{
	int count=0;
	for(int i=0;i<(int)buffer.length();i++)
	if(buffer[i]>=65 && buffer[i]<=90) count++;
	return count;
}
//----- get lower case -----//
int getLowerCase(char *buffer)
{
	int count=0;
	for(int i=0;i<(int)strlen(buffer);i++)
	if(buffer[i]>=97 && buffer[i]<=122) count++;
	return count;
}
int getLowerCase(string &buffer)
{
	int count=0;
	for(int i=0;i<(int)buffer.length();i++)
	if(buffer[i]>=97 && buffer[i]<=122) count++;
	return count;
}


//------------ load MSA in A2M --------------//
long Load_MSA_in_A2M(string &multi_fasta,vector <string> &nam_list,vector <string> &fasta_list, int &totlen)
{
	ifstream fin;
	string buf,temp;
	//-> 1. read
	fin.open(multi_fasta.c_str(), ios::in);
	if(fin.fail()!=0)
	{
		fprintf(stderr,"file %s not found!\n",multi_fasta.c_str());
		exit(-1);
	}
	//-> 2. load
	int first=1;
	int length=0;
	long count=0;
	char command[300000];
	nam_list.clear();
	fasta_list.clear();
	for(;;)
	{
		if(!getline(fin,buf,'\n'))break;
		sprintf(command,"%ld",count);
		string temp=command;
		nam_list.push_back(temp);
		fasta_list.push_back(buf);
		count++;
		//length check
		if(first==1)
		{
			first=0;
			length=(int)buf.length();
		}
		else
		{
			if(length!=(int)buf.length())
			{
				fprintf(stderr,"ini_length %d not equal to cur_length %d at row %ld \n",
					length,(int)buf.length(),count);
				exit(-1);
			}
		}
	}
	//-> 3. return
	totlen=length;
	return count;
}



//-------- read in MSA in a3m format (i.e., normal FASTA with upper/lower) ------------//
//[note]: we set the first sequence as the query sequence,
//        that is to say, all the following sequences should be longer than the first
int Multi_FASTA_Input(string &multi_fasta,vector <string> &nam_list,vector <string> &fasta_list)
{
	ifstream fin;
	string buf,temp;
	//read
	fin.open(multi_fasta.c_str(), ios::in);
	if(fin.fail()!=0)
	{
		fprintf(stderr,"file %s not found!\n",multi_fasta.c_str());
		return -1;
	}
	//load
	int firstlen=0;
	int first=1;
	int count=0;
	int number=0;
	string name;
	string seq;
	nam_list.clear();
	fasta_list.clear();
	for(;;)
	{
		if(!getline(fin,buf,'\n'))break;
		if(buf=="")continue;
		if(buf.length()>=1 && buf[0]=='>')
		{
			name=buf.substr(1,buf.length()-1);
			nam_list.push_back(name);
			count++;
			if(first!=1)
			{
				fasta_list.push_back(seq);
				number++;
				if(number==1)
				{
					firstlen=(int)seq.length();
				}
				else
				{
					int lowlen=getLowerCase(seq);
					int curlen=(int)seq.length()-lowlen;
					if(curlen!=firstlen)
					{
						fprintf(stderr,"length not equal at %s, [%d!=%d] \n",buf.c_str(),curlen,firstlen);
						return -1;
					}
				}
			}
			first=0;
			seq="";
		}
		else
		{
			if(first!=1)seq+=buf;
		}
	}
	//final
	if(first!=1)
	{
		fasta_list.push_back(seq);
		number++;
		if(number==1)
		{
			firstlen=(int)seq.length();
		}
		else
		{
			int lowlen=getLowerCase(seq);
			int curlen=(int)seq.length()-lowlen;
			if(curlen!=firstlen)
			{
				fprintf(stderr,"length not equal at %s, [%d!=%d] \n",buf.c_str(),curlen,firstlen);
				return -1;
			}
		}
	}
	//check
	if(number!=count)
	{
		fprintf(stderr,"num %d != count %d \n",number,count);
		return -1;
	}
	return count;
}

//----- eliminate lower case -------//
void Eliminate_LowerCase(string &instr,string &outstr)
{
	int i;
	outstr.clear();
	for(i=0;i<(int)instr.length();i++)
	{
		if(instr[i]>='a' && instr[i]<='z') continue;
		outstr.push_back(instr[i]);
	}
}

//----- eliminate head/tail gap on query sequence ----//
//-> [note]: we ALWAYS fix the query sequence as the 1st one
void Eliminate_HeadTail_Gap(vector <string> &fasta_list)
{
	int i,k;
	int len=(int)fasta_list[0].length();
	int size=(int)fasta_list.size();
	//-> determine start
	int start=-1;
	for(i=0;i<len;i++)
	{
		if(fasta_list[0][i]!='-')
		{
			start=i;
			break;
		}
	}
	//-> determine end
	int end=-1;
	for(i=len-1;i>=0;i--)
	{
		if(fasta_list[0][i]!='-')
		{
			end=i;
			break;
		}
	}
	//-> judge
	if(start==-1 || end==-1)
	{
		fprintf(stderr,"start %d or end %d not correct \n",start,end);
		exit(-1);
	}
	//-> cut head and tail
	if(start !=0 || end !=len-1)
	{
		for(k=0;k<size;k++)fasta_list[i]=fasta_list[i].substr(start,end-start+1);
	}
}


//========= validate sequence ==========//
int Ori_AA_Map[26]=
{ 0,20,2,3,4,5,6,7,8,20,10,11,12,13,20,15,16,17,18,19,20, 1, 9,20,14,20};
// A B C D E F G H I  J  K  L  M  N  O  P  Q  R  S  T  U  V  W  X  Y  Z
// 0 1 2 3 4 5 6 7 8  9 10 11 12 14 14 15 16 17 18 19 20 21 22 23 24 25

void Validate_Sequence(string &instr,string &outstr)
{
	int i;
	int len=(int)instr.length();
	outstr=instr;
	for(i=0;i<len;i++)
	{
		if(instr[i]=='-')continue;
		char a=instr[i];
		if(a<'A' || a>='Z')
		{
			outstr[i]='X';
			continue;
		}
		int retv=Ori_AA_Map[a-'A'];
		if(retv==20)
		{
			outstr[i]='X';
			continue;
		}
	}
}


//===================== for CDHIT preparation =====================//

//---------- remove gap ------------//
void Remove_Gap(string &in,string &out)
{
	int i;
	int len=(int)in.length();
	out="";
	for(i=0;i<len;i++)
	{
		if(in[i]!='-')out+=in[i];
	}
}

//------------ A2M to non-gap seq ------------//
void A2M_To_NonGapSEQ(vector <string> &in, vector <string> &out)
{
	long i;
	long len=(int)in.size();
	out=in;
	for(i=0;i<len;i++)
	{
		Remove_Gap(in[i],out[i]);
	}
}

//-------- extract CD-HIT fragment --------//
long CD_HIT_StrProc(string &in_str,int &start,int &end,
	long totnum,int totlen)
{
	int i;
	int cur;
	long seq_id;
	string temp;
	int len=(int)in_str.length();
	//get seq_id
	for(i=0;i<len;i++)
	{
		if(in_str[i]==':')break;
	}
	temp=in_str.substr(0,i);
	seq_id=atol(temp.c_str());
	if(seq_id<0 || seq_id>=totnum)return -1;
	//get start
	cur=i+1;
	for(i=cur;i<len;i++)
	{
		if(in_str[i]=='-')break;
	}
	temp=in_str.substr(cur,i-cur);
	start=atoi(temp.c_str());
	if(start<0 || start>=totlen)return -1;
	//get end
	cur=i+1;
	temp=in_str.substr(cur,len-cur);
	end=atoi(temp.c_str());
	if(end<0 || end>=totlen)return -1;
	//return
	return seq_id;
}

//------------- CD-HIT process ---------------//
//[note]: sim_thres should be set to 0.7
//        len_thres should be set to 0 (i.e., no restriction)
//[output]: all clusters with one or more than one members
int CD_HIT_Process_New(	
	vector <string> &out_nam,                           //-> input, OriginID/Range
	vector <string> &out_seq,                           //-> input, ungapped fragments
	double sim_thres,double len_thres,
	vector <vector <string> > & output, vector <int> & center )
{
	//-> 0. run CD-HIT
//	int sys_retv;
//	char command[30000];
//	sprintf(command,"./cd-hit -i %s -o %s -c %lf -s %lf -M 0 -T 0 -d 0 1> ws1 2> ws2",
//		infile.c_str(),outname.c_str(),sim_thres,len_thres);
//	sys_retv=system(command);

	//-> 1. initial CDHIT
	Options options;
	SequenceDB seq_db;
	options.cluster_thd=sim_thres;   // -c sim_thres
	options.diff_cutoff=len_thres;   // -s len_thres
	options.max_memory=0;            // -M 0
	options.des_len=0;               // -d 0
#ifndef NO_OPENMP
	int cpu = omp_get_num_procs();   // -T 0
	options.threads = cpu;
#endif
	options.Validate();
	InitNAA( MAX_UAA );
	options.NAAN = NAAN_array[options.NAA];
	seq_db.NAAN = NAAN_array[options.NAA];

	//-> 2. run CDHIT to get all clusters with more than one members
	seq_db.Read_From_String( options, out_nam, out_seq );
	seq_db.SortDivide( options );
	seq_db.DoClustering( options );
	seq_db.WriteExtra1D_To_String( options, output,center);

	//return
	CDHIT_CleanUpTempFiles();
	return (int)center.size();
}

//--------- from CD-HIT cluster, construct a cluster matrix ---------//
void Cluster_To_Matrix(vector <vector <string> > & strnam, vector <int> & center,
		map<long, int > & cluster_mapping, long input_size, int totlen )
{
	map<long, int >::iterator iter;
	cluster_mapping.clear();
	int i,k,l;
	int cluster_size=(int)center.size();
	//-> process each cluster
	for(i=0;i<cluster_size;i++)
	{
		int member=(int)strnam[i].size();
		//-> process pairwise cluster mapping
		for(k=0;k<member;k++)
			for(l=0;l<member;l++)
			{
				if(k==l)continue;
				long seqid_k=atol(strnam[i][k].c_str());
				long seqid_l=atol(strnam[i][l].c_str());
				//generate uniq_ID
				long uniq_ID=input_size*seqid_k+seqid_l;
				//add to map
				iter = cluster_mapping.find(uniq_ID);
				if(iter != cluster_mapping.end())continue;
				cluster_mapping.insert(map < long, int >::value_type(uniq_ID, i));
			}
	}
}


//================== for filter process ========================//

//------- calculate similarity ---------//
void compute_similarity(const string &a, const string &b, double &sim1, double &sim2)
{
	if(a.length() != b.length() )
		exit(-2);
	int len = 0, match=0;
	int len1 = 0, len2 = 0;
	for(size_t i=0; i<a.length(); ++i)
	{
		if(a[i] != '-')len1++;
		if(b[i] != '-')len2++;
		if(a[i] == '-' || b[i] == '-')
			continue;
		if(a[i] == b[i])
			++ match;
		++ len;
	}
	sim1=1.0*match/len1;
	sim2=1.0*match/len2;
}

//-------- calculate coverage --------//
double compute_coverage(const string &in)
{
	int gap=0;
	for(int i=0;i<(int)in.length();i++)
	{
		if(in[i]=='-')gap++;
	}
	return 1.0*gap/(int)in.length();
}



//---------- usage ---------//
void Usage() 
{
	fprintf(stderr,"Version: 1.01 \n");
	fprintf(stderr,"Self_Filter -i/I a3m(a2m)_input -o/O a3m(a2m)_output [-s sim_thres] [-d coverage] [-c cdhit] \n");
	fprintf(stderr,"Usage : \n\n");
	fprintf(stderr,"-i/I aXm_input  : Input MSA file in A3M/A2M format. \n\n");
	fprintf(stderr,"-o/O aXm_output : Output filterd MSA file in A3M/A2M format. \n\n");
	fprintf(stderr,"-s sim_thres    : Similarity threshold to query sequence. \n");
	fprintf(stderr,"                  (by default, sim_thres = 0.95, should between 0.0 to 1.0) \n\n");
	fprintf(stderr,"-d coverage     : Coverage threshold to query sequence. \n");
	fprintf(stderr,"                  (by default, coverage = 0.5, should between 0.0 to 1.0) \n\n");
	fprintf(stderr,"-c cdhit        : Run CD-HIT or not to filter the redundant sequences. (default = 1) \n\n");
}

//------------ main ---------------//
int main(int argc, char *argv[])
{
	//------ Self_Filter -------//
	if(argc<3)
	{
		Usage();
		exit(-1);
	}
	string axm_input="";
	string axm_output="";
	int axm_input_key=0;
	int axm_output_key=0;
	double sim_thres=0.95;     //-> Similarity threshold to filter other sequences (default: 0.95)
	double coverage=0.5;       //-> Coverage threshold to filter other sequences (default: 0.5)
	int cdhit_filter=1;        //-> run cdhit to filter the redundant sequences. (default: 1)

	//command-line arguments process
	extern char* optarg;
	char c = 0;
	while ((c = getopt(argc, argv, "i:I:o:O:s:d:c:")) != EOF) {
		switch (c) {
		case 'i':
			axm_input = optarg;
			axm_input_key = 0;  //-> 0 for a3m
			break;
		case 'I':
			axm_input = optarg;
			axm_input_key = 1;  //-> 1 for a2m
			break;
		case 'o':
			axm_output = optarg;
			axm_output_key = 0; //-> 0 for a3m
			break;
		case 'O':
			axm_output = optarg;
			axm_output_key = 1; //-> 1 for a2m
			break;
		case 's':
			sim_thres = atof(optarg);
			break;
		case 'd':
			coverage = atof(optarg);
			break;
		case 'c':
			cdhit_filter = atoi(optarg);
			break;
		default:
			Usage();
			exit(-1);
		}
	}

	//check arguments
	if(axm_input=="")
	{
		fprintf(stderr,"a3m(a2m)_input should be specified. \n");
		exit(-1);
	}
	if(axm_output=="")
	{
		fprintf(stderr,"a3m(a2m)_output should be specified. \n");
		exit(-1);
	}
	if( sim_thres<0 || sim_thres>1  )
	{
		fprintf(stderr,"sim_thres %lf should between 0.0 and 1.0 \n",sim_thres);
		exit(-1);
	}
	if( coverage<0 || coverage>1  )
	{
		fprintf(stderr,"coverage %lf should between 0.0 and 1.0 \n",coverage);
		exit(-1);
	}


	//----- general data structure ---//
	vector <string> nam_list;
	vector <string> fasta_list;
	vector <string> fasta_list_orig;
	long msa_num;
	int totlen;


	//----- load input A3M -------//
	if(axm_input_key==0)
	{
		msa_num=Multi_FASTA_Input(axm_input,nam_list,fasta_list_orig);
		//length check
		int first=1;
		for(long i=0;i<msa_num;i++)
		{
			//get sequence
			string seq;
			Eliminate_LowerCase(fasta_list_orig[i],seq);
			string outseq;
			Validate_Sequence(seq,outseq);
			//length check
			if(first==1)
			{
				totlen=(int)outseq.length();
				first=0;
			}
			else
			{
				if(totlen!=(int)outseq.length())
				{
					fprintf(stderr,"ini_length %d not equal to cur_length %d at pos %ld \n",
						totlen,(int)outseq.length(),i);
					exit(-1);
				}
			}
			//push back
			fasta_list.push_back(outseq);
		}
	}
	//----- load input A2M -------//
	else
	{
		msa_num=Load_MSA_in_A2M(axm_input,nam_list,fasta_list,totlen);
		fasta_list_orig=fasta_list;
	}
	//----- eliminate head/tail gap ----//
	Eliminate_HeadTail_Gap(fasta_list);

	//======== start CD-HIT filter =======//
	vector <int> filtered(msa_num,1);    //-> 1 for keep
	if(cdhit_filter==1)
	{
		//-> cd-hit pre-process
		//--| remove gap in the input sequences
		vector <string> fasta_seq=fasta_list;
		A2M_To_NonGapSEQ(fasta_list, fasta_seq);
		//--| use a unique ID for input names (starting from 0)
		vector <string> out_nam=nam_list;
		if(axm_input_key==0)
		{
			#pragma omp for schedule(dynamic)
			for(long i=0;i<msa_num;i++)
			{
				stringstream ss;
				ss << i;
				out_nam[i] = ss.str();
			}
		}
		//-> run cd-hit to generate cluster_matrix
		double sim_thres_cdhit=sim_thres;
		double len_thres=0;
		vector <vector <string> > strnam;
		vector <int> center;
		CD_HIT_Process_New(out_nam,fasta_seq,sim_thres_cdhit,len_thres,
			strnam,center);
		//-> cd-hit filter
		for(int i=0;i<(int)center.size();i++)
		{
			for(int k=0;k<(int)strnam[i].size();k++)
			{
				if(k==center[i])continue;
				long seqid_k=atol(strnam[i][k].c_str());
				filtered[seqid_k]=0;
			}
		}
	}
	filtered[0]=1;


	//======== start filter procedure ============//
	#pragma omp parallel for schedule(dynamic)
	for(long i=1;i<msa_num;i++)
	{
		//-> skip filtered members
		if(filtered[i]==0)continue;
		//-> filter by coverage
		double cover=compute_coverage(fasta_list[i]);
		if(cover>coverage)filtered[i]=0;
		else
		{
			//-> filter by similarity
			double sim1,sim2;
			compute_similarity(fasta_list[0], fasta_list[i],sim1,sim2);
			if(sim2>sim_thres)filtered[i]=0;
		}
	}

	//------- output sampled K sequence -----//
	FILE *fp=fopen(axm_output.c_str(),"wb");
	for(long i=0;i<msa_num;i++)
	{
		if(filtered[i]==0)continue;
		if(axm_output_key==0)fprintf(fp,">%s\n",nam_list[i].c_str());
		fprintf(fp,"%s\n",fasta_list_orig[i].c_str());
	}
	fclose(fp);

	//======== exit ========//
	exit(0);
}
