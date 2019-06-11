#include "../common/APxLinkData.hh"
//#include <ap_int.h>

using namespace std;

int main(int argn, char *argp[]) {
  APxLinkData link_in(24);

  for (size_t i = 0; i < 6; i++) {
    for (size_t k = 0; k < 24; k++)
      link_in.add(i, k, {0x00, i});
  }

  link_in.write("test_in.txt");

  return 0;
}
