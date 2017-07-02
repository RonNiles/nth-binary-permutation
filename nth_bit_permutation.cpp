/* bit permutations:

   What if you took all of the possible integers with a specific number of set bits and arranged them in 
   ascending order? These are "binary permutations" or "bit permutations".
   These routines use Pascal's triangle to get the nth bit permutation and conversely to get the 
   position (or rank) of a specific binary permutation.
*/

#include <iostream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <climits>
#include <stdexcept>
#include <bitset>

/* the maximum number of bits to be used. Pascal's triangle will be expanded according to this value */
#define MAXBITS 32

/* pascal's triangle - needed for indexing bitmap permutations */
std::vector<std::vector<unsigned> > pascalt;
void make_pascal_triangle(void) {
   pascalt.clear();
   pascalt.resize(MAXBITS+1);
   pascalt[0].push_back(1);
   for (unsigned i=1; i<=MAXBITS; ++i) {
      pascalt[i].push_back(pascalt[i-1][0]);
      for (unsigned j=1; j<pascalt[i-1].size(); ++j) {
         /* In Pascal's triangle, each entry is the sum of the two numbers above it in the previous row */
         pascalt[i].push_back(pascalt[i-1][j-1] + pascalt[i-1][j]);
      }
      pascalt[i].push_back(pascalt[i-1].back());
   }
}

/* given a bitmap where ntotbits are set out of nsetbits, return the rank */
unsigned rank_from_permutation(unsigned bitmap, unsigned ntotbits, unsigned nsetbits) {
   unsigned total = 0;
   unsigned row = ntotbits;
   unsigned col = row - nsetbits;
   unsigned thisbit = 1 << (ntotbits-1);
   for(;;) {
      if (pascalt[row][col] == 1) {
         break;
      }
      row--;
      if (bitmap & thisbit) {
         total += pascalt[row][col-1];
      } else {
         col--;
      }
      thisbit >>= 1;
   }
   return total;
}

/* given a rank from zero through (ntotbits choose nsetbits) minus 1, return the nth binary permutation 
   as a bitmap */
unsigned permutation_from_rank(unsigned rank, unsigned ntotbits, unsigned nsetbits) {
   unsigned res = 0;
   unsigned row = ntotbits;
   unsigned col = row - nsetbits;
   unsigned thisbit = 1 << (ntotbits-1);
   for(;;) {
      if (row-- == 0) break;
      if (col > 0 && rank < pascalt[row][col-1]) {
         --col;
      } else {
         if (col) rank -= pascalt[row][col-1];
         res |= thisbit;
      }
      thisbit >>= 1;
   }
   return res;
}

void bit_permutation_test(void) {
   /* go through all possible input values. The inner loop generates the bit permutations in order
      using Sean Eron Anderson's bit-twiddling hack. Then we check that the bit permutation generates
      the expected rank, and that the rank generates the expected bit permutation */
   std::cout << "Testing all possible inputs for values up to 32-bits" << std::endl;
   for (unsigned ntotbits = 0; ntotbits <= MAXBITS; ++ntotbits) {
      for (unsigned nsetbits = 0; nsetbits <= ntotbits; ++nsetbits) {
         std::cout << ntotbits << ":" << nsetbits << " bits   \r" << std::flush;
         std::string nd;
         nd.append(ntotbits - nsetbits, '0');
         nd.append(nsetbits, '1');
         unsigned bitperm = (1ULL << nsetbits) - 1;
         unsigned count = 0;
         do {
            unsigned bits = 0;
            unsigned val = 1;
            for (unsigned i=0; i<ntotbits; ++i) {
               if (nd[ntotbits-i-1] == '1') bits += val;
               val <<= 1;
            }
            if (bitperm != bits) throw std::runtime_error("bitperm is not equal to bits");
            /* The folowing code is the "next bit permutation" routine from the "Bit Twiddling Hacks"
               By Sean Eron Anderson at Stanford University */
            unsigned t = bitperm | (bitperm -1);
            unsigned v = bitperm;
            unsigned ctz = 0;
            if (v) {
               v = (v ^ (v - 1)) >> 1;  // Set v's trailing 0s to 1s and zero rest
               while (v) {
                  ++ctz;
                  v >>= 1;
               }
            } else {
               ctz = CHAR_BIT * sizeof(v);
            }
            bitperm = (t+1) | (((~t & -~t) -1) >> (ctz + 1));
            unsigned rankval = rank_from_permutation(bits, ntotbits, nsetbits);
            if (rankval != count) throw std::runtime_error("rankval is not equal to rank");
            unsigned pfr = permutation_from_rank(count, ntotbits, nsetbits);
            if (pfr != bits) {
               throw std::runtime_error("permutation_from_rank is not equal to bits");
            }
            ++count;
         } while(next_permutation(nd.begin(), nd.end()));
      }
   }
   std::cout << "Test complete." << std::endl;
   return;
}

int main(int argc, char *argv[]) {
   try {
      make_pascal_triangle();
      std::cout << "The first 20 binary permutations of 5 set bits out of 8 total bits are " << std::endl;
      for (unsigned i=0; i<20; ++i) {
         std::cout << std::setw(5) << i+1 << ": "  << std::bitset<8>(permutation_from_rank(i, 8,5)) << std::endl;
      }
      bit_permutation_test();
   } catch (std::exception &x) {
      std::cout << std::endl << x.what() << std::endl;
      return 1;
   }
   return 0;
}
