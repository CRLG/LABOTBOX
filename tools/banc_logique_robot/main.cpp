#include <QCoreApplication>
#include <cstdio>
#include <cstring>
#include "banc.h"
#include "scenarios.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    if (argc < 2) {
        fprintf(stderr, "usage : %s <librobotlogic_xxx.so> [--trace <secondes> | --avant]\n", argv[0]);
        return 2;
    }
    Banc banc(argv[1]);
    if (!banc.estCharge()) return 2;

    if (argc >= 4 && strcmp(argv[2], "--trace") == 0) {
        trace_match(banc, atoi(argv[3]));
        return 0;
    }

    scenarios_mode_avant(argc >= 3 && strcmp(argv[2], "--avant") == 0);
    scenarios_etape0(banc);
    printf("\n%d verification(s), %d echec(s)\n", banc.verifications(), banc.echecs());
    return banc.echecs() == 0 ? 0 : 1;
}
