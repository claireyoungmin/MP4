#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <list>
#include <vector>

using namespace std;

class packet{
private:
    string binSource;
    string binDest;
    string binSeq;
    string binDataLen;
public:
    packet();
    string source;
    string dest;
    string data;
    short seq;
    string checkSum;
    int dataLen;
    int binToInt(string, int);
    void divPacket(string);
    bool operator<(packet);
    bool operator==(packet);
    bool check(string);
    template <class T> string toString(T value);
};

packet::packet(){
    source = "";
    dest = "";
    seq = 0;
    checkSum = "";
    data = "";
    dataLen = 0;
};

int packet::binToInt(string bin, int length){   //converts binary to Integer
    int val=0, power=1;
    for(int i=length; i>=0; i--){
        val+=power*((int)bin[i]-48);
        power*=2;
    }
    return val;
}

template<class T>
string packet::toString(T val){             // converts any data type T to string
	stringstream ss;
    ss << val;
    return ss.str();
}

void packet::divPacket(string bin){         //divides the packet to its corresponding parts
    binSource = bin.substr(0, 32);
    binDest = bin.substr(32, 32);
    binSeq = bin.substr(64, 16);
    checkSum = bin.substr(80, 16);
    binDataLen = bin.substr(96, 16);
    stringstream ss;
    string tmp;
    tmp = binDataLen;
    dataLen = binToInt(tmp, 15);
    checkSum = toString(binToInt(checkSum, 15));    // converts Checksum to Integer which later on is converted to String

    if(!check(bin))
        data = "[line corrupted]";
    else{
        data = "";
        int i;
        for(dataLen/=8,i=0;i<dataLen;i++)
            data=data+toString((char)strtoull(bin.substr(112+i*8,8).c_str(),NULL,2));
    }

    for (int i=0; i<4; i++){        //source address w/ "."
        tmp = binSource.substr(i*8, 8);
        ss << binToInt(tmp, 7);
        source = source+ss.str();
        if(i<3)
            source = source + ".";
        ss.str("");
    }

    for (int i=0; i<4; i++){    //destination address with "."
        tmp = binDest.substr(i*8, 8);
        ss << binToInt(tmp, 7);
                dest=dest+ss.str();
        if(i<3)
            dest=dest+".";
        ss.str("");
    }
    tmp=binSeq;
    seq=(short)strtoull(bin.substr(64, 16).c_str(), NULL, 2);
}

bool packet::operator<(packet m){       //operator overloading for sort fn
    if (abs(seq)<abs(m.seq))            //absolute value is necessary to make sure the last packet is still at the last.
        return true;
    else
        return false;
}

bool packet::operator==(packet m){      // operator overloading for unique fn
    if (seq==m.seq)
        return true;
    else
        return false;
}

bool packet::check(string bin){

	bin.replace(80,16,"0000000000000000"); // replace checksum with zeros.
	vector<int> binNum;

	for(int i=0; bin[i*16]!='\0'; i++){ // store 16-bit values.
		binNum.push_back(strtoull(bin.substr(i*16,16).c_str(),NULL,2));
    }

	int n=binNum.size(), sum = 0;

	for(int i=0; i<n; sum+=binNum[i],i++)
		while(sum>=65536)        //2^16 = 65536
			sum-=65535;
	while(sum>=65536)
		sum-=65535;
	sum=65535-sum;
	if(sum==atoi(checkSum.c_str()))
		return true;             // returns true if checksum matches
	return false;                // returns false meaning we have a corrupted packet
}

int main(){
    string str;
    ifstream fp("cases_random.in");
    list<list<packet> > poemList;
    list<list<packet> >::iterator it;
    int flag=0;
    if(fp.is_open()){
        while(!fp.eof()){
            packet a;
            fp>>str;
            if(str.size()<112)
                flag=1;
            else{
                a.divPacket(str);
                    if(poemList.empty()){
                        list<packet> poem;          //creates list of packets when poemList is empty
                        poem.push_back(a);
                        poemList.push_back(poem);
                    }
                    else{
                        for(it=poemList.begin(); it!=poemList.end(); it++){
                            if(it->front().source.compare(a.source)==0 && it->front().dest.compare(a.dest)==0){     //sorts out packets from the same poem
                                it->push_back(a);
                                break;
                            }
                        }
                        if(it==poemList.end()){
                            list<packet> poem;
                            poem.push_back(a);
                            poemList.push_back(poem);
                        }
                    }
            }
        }

    }
    for(it = poemList.begin(); it != poemList.end(); it++)
        it->sort();             // sorts the lines in each poem
    for(it = poemList.begin(); it != poemList.end(); it++)
        it->unique();            // unique function only works if packets are sorted
    ofstream output("poems.in");
    for(it = poemList.begin(); it != poemList.end(); it++){
        int i=0;
        for(list<packet> :: iterator iter = it->begin(); iter!=it->end(); i++){
            if(abs(iter->seq)==i){
                 output << iter->data<<endl;
                 iter++;
            }
            else{
                if(i==0)
                    output<<"[title missing]" << endl;
                 else
                    output << "[line missing]" << endl;
            }
            if(i==0)
                output << iter->source << "/" << iter->dest << endl;
        }
         output << "--------------------------------------------------------------------------------" << endl << endl;
    }
    fp.close();
  return 0;
}
