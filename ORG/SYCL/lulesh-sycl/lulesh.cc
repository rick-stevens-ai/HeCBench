#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sycl/sycl.hpp>
#include "lulesh.h"

struct cmdLineOpts ParseCommandLineOptions(int argc, char *argv[])
{
    struct cmdLineOpts opts;
    opts.its = 9999999;
    opts.nx  = 30;
    opts.numReg = 11;
    opts.numFiles = 50;
    opts.showProg = 0;
    opts.quiet = 0;
    opts.viz = 0;
    opts.balance = 1;
    opts.cost = 1;
    opts.iteration_cap = 0;

    for(int i=1; i<argc; ++i) {
        if(strcmp(argv[i], "-i") == 0) {
            if(i+1 < argc) opts.its = atoi(argv[++i]);
        }
        else if(strcmp(argv[i], "-s") == 0) {
            if(i+1 < argc) opts.nx = atoi(argv[++i]);
        }
        else if(strcmp(argv[i], "-r") == 0) {
            if(i+1 < argc) opts.numReg = atoi(argv[++i]);
        }
        else if(strcmp(argv[i], "-f") == 0) {
            if(i+1 < argc) opts.numFiles = atoi(argv[++i]);
        }
        else if(strcmp(argv[i], "-p") == 0) {
            if(i+1 < argc) opts.showProg = atoi(argv[++i]);
        }
        else if(strcmp(argv[i], "-q") == 0) {
            opts.quiet = 1;
        }
        else if(strcmp(argv[i], "-v") == 0) {
            if(i+1 < argc) opts.viz = atoi(argv[++i]);
        }
        else if(strcmp(argv[i], "-h") == 0) {
            printf("Usage: %s [opts]\n", argv[0]);
            printf(" where [opts] is one or more of:\n");
            printf(" -i <iterations> : number of cycles to run\n");
            printf(" -s <size>       : length of cube mesh along side\n");
            printf(" -r <numregions> : Number of distinct regions (def: 11)\n");
            printf(" -f <numfiles>   : Number of files to create (def: 50)\n");
            printf(" -p <numprocs>   : Processors along each dimension (def: 1)\n");
            printf(" -q              : Quiet mode - suppress progress statements\n");
            printf(" -v <visLevel>   : Control visualization\n");
            printf(" -h              : This message\n");
            exit(0);
        }
    }
    return opts;
}

int main(int argc, char *argv[])
{
    struct cmdLineOpts opts = ParseCommandLineOptions(argc, argv);
    
    try {
        Domain *domain = new Domain(1, 0, 0, 0, opts.nx, 0, 0, opts.balance, opts.cost);

        // Print initial mesh and device info
        DumpMeshMetadata(*domain, "Initial Mesh");
        PrintSYCLInfo(*domain);

        // Run the main simulation loop
        while(domain->time() < domain->stoptime() && domain->cycle() < opts.its) {
            domain->TimeIncrement();
            domain->LagrangeElements();
            domain->CalcTimeConstraintsForElems();

            // Print progress if requested
            if (!opts.quiet && (opts.showProg != 0)) {
                printf("cycle = %d, time = %e\n", domain->cycle(), domain->time());
            }
        }

        delete domain;
    }
    catch (const sycl::exception& e) {
        std::cerr << "SYCL exception caught: " << e.what() << std::endl;
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << "Standard exception caught: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Unknown exception caught" << std::endl;
        return 1;
    }

    return 0;
}

void Domain::TimeIncrement()
{
    // Simple time increment for now
    Real_t targetdt = stoptime() - time();

    if (targetdt > dtfixed()) targetdt = dtfixed();

    Real_t ratio = Real_t(1.);
    if (dtcourant() < targetdt) {
        targetdt = dtcourant();
        ratio = targetdt / dtcourant();
    }

    if (dthdryo() < targetdt) {
        targetdt = dthdryo();
        ratio = targetdt / dthdryo();
    }

    m_deltatime = targetdt;
    m_time += targetdt;
    m_cycle++;
}

void Domain::LagrangeElements()
{
    // Basic Lagrangian step - to be expanded with SYCL kernels
    try {
        auto& queue = *q_device;
        queue.submit([&](sycl::handler& h) {
            auto p = m_p.data();
            h.parallel_for(sycl::range<1>(m_numElem), [=](sycl::id<1> idx) {
                p[idx[0]] += Real_t(0.1);
            });
        });
        queue.wait();
    }
    catch (const sycl::exception& e) {
        std::cerr << "SYCL kernel exception: " << e.what() << std::endl;
        throw;
    }
}

void Domain::CalcTimeConstraintsForElems()
{
    // Basic time constraints calculation
    m_dtcourant = Real_t(1.0e+20);
    m_dthydro = Real_t(1.0e+20);

    for (Index_t i = 0; i < m_numElem; ++i) {
        if (m_vdov[i] != Real_t(0.)) {
            Real_t dtf = m_ss[i] * m_ss[i];
            if (dtf < m_dtcourant) {
                m_dtcourant = dtf;
            }
        }
    }

    m_dtcourant = Real_t(0.7) * sycl::sqrt(m_dtcourant);
    m_dthydro = Real_t(0.7) * m_dthydro;
}
