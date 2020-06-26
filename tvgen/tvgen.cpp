#include "../common/APxLinkData.hh"
#include <ap_int.h>
#include <bitset>

using namespace std;

/* ECAL crystal object definition */
struct Crystal {
  Crystal() : energy(0), timing(0), spike(false) {};

  Crystal(ap_uint<14> i) {
    this->energy = i;
    this->timing = i >> 10;
    this->spike = i >> 13;
  }

  inline operator ap_uint<14>() {
    return  ((uint16_t)(this->spike) << 13) |
      ((uint16_t)(this->timing) << 10) |
      this->energy;
  }

#ifndef __SYNTHESIS__
  string toString() {
    return "energy = " + to_string(this->energy) + ", timing = " + to_string(this->timing) + ", spike = " + to_string(this->spike);
  }
#endif

  ap_uint<10> energy;
  ap_uint<3> timing;
  bool spike;
};

/* Tower object definition */
struct Tower {
  Tower() {
#pragma HLS ARRAY_PARTITION variable=crystals complete dim=0
    for (size_t i = 0; i < 5; i++) {
#pragma LOOP UNROLL
      for (size_t j = 0; j < 5; j++) {
#pragma LOOP UNROLL
	this->crystals[i][j] = Crystal();
      }
    }
  }

#ifndef __SYNTHESIS__
  string toString() {
    ostringstream str;
    for (size_t i = 0; i < 5; i++) {
      for (size_t k = 0; k < 5; k++) {
	str << setfill('0') << setw(4) << hex << this->crystals[i][k].energy << " ";
      }
      str << endl;
    }

    return str.str();
  }
#endif

  // Packs a tower object into 6 64-bit words, useful for link formatting
  void pack(ap_uint<64> out[6]) {

    for (size_t i = 0; i < 6; i++) out[i] = 0;

    int ncrys=0;
    for(size_t i=0; i<5; i++)
      for(size_t j=0; j<5; j++){
	if(this->crystals[i][j].energy>0)
	  cout<<"Crys#"<<ncrys<<": ["<<i<<"]["<<j<<"]: "<<this->crystals[i][j].energy<<endl;;
	ncrys++;
      }

    out[0].range(13, 0)  = this->crystals[0][0].energy; //0 (14)
    out[0].range(27, 14) = this->crystals[0][1].energy; //1 (14)
    out[0].range(41, 28) = this->crystals[0][2].energy; //2 (14)
    out[0].range(55, 42) = this->crystals[0][3].energy; //3 (14)
    out[0].range(63, 56) = this->crystals[0][4].energy & 0xFF; //4 (8)

    out[1].range( 5,  0) = this->crystals[0][4].energy >>8 & 0x3F; //4 (6)
    out[1].range(19,  6) = this->crystals[1][0].energy; //5 (14)
    out[1].range(33, 20) = this->crystals[1][1].energy; //6 (14)
    out[1].range(47, 34) = this->crystals[1][2].energy; //7 (14)
    out[1].range(61, 48) = this->crystals[1][3].energy; //8 (14)
    out[1].range(63, 62) = this->crystals[1][4].energy & 0x3; //9 (2)

    out[2].range(11,  0) = this->crystals[1][4].energy >>2 & 0xFFF; //9 (12)
    out[2].range(25, 12) = this->crystals[2][0].energy; //10 (14)
    out[2].range(39, 26) = this->crystals[2][1].energy; //11 (14)
    out[2].range(53, 40) = this->crystals[2][2].energy; //12 (14)
    out[2].range(63, 54) = this->crystals[2][3].energy & 0x3FF; //13 (10)

    out[3].range( 3,  0) = this->crystals[2][3].energy >>10 & 0xF; //13 (4)
    out[3].range(17,  4) = this->crystals[2][4].energy; //14 (14)
    out[3].range(31, 18) = this->crystals[3][0].energy; //15 (14)
    out[3].range(45, 32) = this->crystals[3][1].energy; //16 (14)
    out[3].range(59, 46) = this->crystals[3][2].energy; //17 (14)
    out[3].range(63, 60) = this->crystals[3][3].energy & 0xF; //18 (4)

    out[4].range( 9,  0) = this->crystals[3][3].energy >>4 & 0x3FF; //18 (10)
    out[4].range(23, 10) = this->crystals[3][4].energy; //19 (14)
    out[4].range(37, 24) = this->crystals[4][0].energy; //20 (14)
    out[4].range(51, 38) = this->crystals[4][1].energy; //21 (14)
    out[4].range(63, 52) = this->crystals[4][2].energy & 0xFFF; //22 (12)

    out[5].range( 1,  0) = this->crystals[4][2].energy >> 12 & 0x3; //22 (2)
    out[5].range(15,  2) = this->crystals[4][3].energy; //23 (14)
    out[5].range(29, 16) = this->crystals[4][4].energy; //24 (14)
    out[5].range(63, 30) = 0; //EMPTY (34)

    for(size_t kk=0; kk< 6;  kk++)
      cout<<std::setw(20)<<out[kk]<<"   "<<std::bitset<64>(out[kk])<<std::endl;
  }

  Crystal crystals[5][5];
};

#define ETA 17
#define PHI 2

int main(int argn, char *argp[]) {

  Tower towers[ETA][PHI];

  //-  int counter = 0;
  //-for(size_t ieta=0; ieta<5; ieta++){
  //-   for(size_t iphi=0; iphi<5; iphi++){
  //-      counter++;
  //-      towers[0][0].crystals[ieta][iphi].energy=counter;
  //-      cout<<"crystals["<<ieta<<"]["<<iphi<<"] : "<<towers[0][0].crystals[ieta][iphi].energy<<std::endl;
  //-   }
  //-}
  //-      cout<<"----------"<<std::endl;

  //-| //set - 1 
  //-| towers[0][0].crystals[2][3].energy = 11;
  //-| towers[0][0].crystals[2][4].energy = 22;
  //-| towers[0][1].crystals[2][0].energy = 50;
  //-| towers[1][0].crystals[0][3].energy = 44;
  //-| towers[1][1].crystals[3][1].energy = 55;
  //-| towers[2][0].crystals[4][4].energy = 66;
  //-| towers[2][1].crystals[3][3].energy = 77;
  //-| towers[2][1].crystals[3][4].energy = 88;
  //-| towers[3][0].crystals[2][1].energy = 99;
  //-| towers[4][1].crystals[3][1].energy = 95;
  //-| towers[5][0].crystals[3][3].energy = 8;
  //-| towers[6][1].crystals[4][0].energy = 25;
  //-| towers[6][0].crystals[4][3].energy = 10;
  //-| towers[6][0].crystals[4][4].energy = 36;
  //-| towers[7][0].crystals[0][4].energy = 14;
  //-| towers[7][1].crystals[4][3].energy = 70;
  //-| towers[8][0].crystals[3][1].energy = 35;
  //-| towers[8][1].crystals[2][3].energy = 26;
  //-| towers[9][0].crystals[3][1].energy = 43;
  //-| towers[9][0].crystals[2][2].energy = 204;
  //-| towers[9][1].crystals[0][3].energy = 30;
  //-| towers[12][0].crystals[1][1].energy = 170;
  //-| towers[14][1].crystals[4][2].energy = 123;

  // set - 2
  towers[0][0].crystals[1][1].energy = 3;
  towers[0][0].crystals[1][3].energy = 7;
  towers[0][0].crystals[2][2].energy = 13;
  towers[0][0].crystals[2][3].energy = 26;
  towers[0][0].crystals[3][3].energy = 9;
  towers[0][1].crystals[2][4].energy = 24;
  towers[0][1].crystals[3][3].energy = 13;
  towers[0][1].crystals[3][4].energy = 43;
  towers[1][1].crystals[3][1].energy = 55;
  towers[1][1].crystals[3][2].energy = 11;
  towers[3][0].crystals[4][3].energy = 73;
  towers[4][0].crystals[3][4].energy = 16;
  towers[4][0].crystals[4][4].energy = 36;
  towers[4][1].crystals[4][0].energy = 14;
  towers[5][0].crystals[0][4].energy = 4;
  towers[5][1].crystals[3][3].energy = 11;
  towers[5][1].crystals[4][3].energy = 34;
  towers[6][1].crystals[0][3].energy = 22;
  towers[7][0].crystals[2][2].energy = 37;
  towers[9][0].crystals[4][3].energy = 18;
  towers[10][0].crystals[0][3].energy = 38;
  towers[11][1].crystals[2][1].energy = 15;
  towers[11][1].crystals[3][1].energy = 45;
  towers[12][1].crystals[4][3].energy = 27;
  towers[13][0].crystals[3][4].energy = 16;
  towers[14][0].crystals[1][4].energy = 40;
  towers[14][1].crystals[1][0].energy = 51;
  towers[15][0].crystals[4][1].energy = 32;
  towers[16][0].crystals[0][1].energy = 21;
  towers[16][1].crystals[3][4].energy = 12;

  ap_uint<64> packed[ETA][PHI][6];
  int nlink=0;
  for (size_t k = 0; k < PHI; k++) {
    for (size_t i = 0; i < ETA; i++) {
      std::cout<<"++++ LINK # "<<nlink<<"                           ------- ["<<i<<"]["<<k<<"] -------"<<endl;
      towers[i][k].pack(packed[i][k]);
      std::cout<<endl;
      nlink++;
    }
  }


  APxLinkData link_in(ETA*PHI);

  for (size_t i = 0; i < 6; i++) {
    for (size_t k = 0; k < ETA*PHI; k++) {
      size_t phi = k / ETA;
      size_t eta = k % ETA;
      link_in.add(i, k, {0x00, packed[eta][phi][i]});
    }
  }

  link_in.write("test_in.txt");

  return 0;
}