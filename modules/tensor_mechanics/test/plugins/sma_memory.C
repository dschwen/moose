/***************************************************************************************
**  UMAT, FOR ABAQUS/STANDARD INCORPORATING ISOTROPIC ELASTICITY                      **
***************************************************************************************/

#include "SMAAspUserSubroutines.h"
#include "MooseError.h"
// #include "AbaqusUtils.h"
#include "libmesh/threads.h"

extern "C" void
uexternaldb_(
    int * LOP, int * /* LRESTART */, Real /* TIME */[], Real * /* DTIME */, int * KSTEP, int * KINC)
{
  Threads::spin_mutex::scoped_lock lock(Threads::spin_mtx);
  Moose::out << "======================= Thread " << get_thread_id() << " LOP=" << *LOP
             << " ======================\n";

  bool threaded = getnumthreads() > 1 || true;
  switch (*LOP)
  {
    // beginning of the simulation
    case 0:
    {
      Real * fa = threaded ? SMALocalFloatArrayCreate(7, 5, 1.1) : SMAFloatArrayCreate(7, 5, 1.1);
      Real fasum = 0.0;
      for (unsigned int i = 0; i < 5; ++i)
        fasum += fa[i];

      if (threaded && get_thread_id() % 2 == 1)
        SMALocalFloatArrayCreate(16, 15, 2.2);
      else
        SMAFloatArrayCreate(16, 15, 2.2);

      Moose::out << "Got data address " << fa << '\n';

      // AbaqusUtils::smaDebug();

      // test wrong handle error based on kstep setting
      Real * fa2 = threaded ? SMALocalFloatArrayAccess(*KSTEP == 1 ? 2 : 7)
                            : SMAFloatArrayAccess(*KSTEP == 1 ? 2 : 7);
      // if (fa != fa2)
      //   mooseError("Mismatching float pointers ", fa, " != ", fa2);

      int * ia = threaded ? SMALocalIntArrayCreate(67, 39, 9) : SMAIntArrayCreate(67, 39, 9);
      int iasum = 0;
      for (unsigned int i = 0; i < 39; ++i)
        iasum += ia[i];

      // test wrong handle error based on kstep setting
      int * ia2 = threaded ? SMALocalIntArrayAccess(*KSTEP == 2 ? 13 : 67)
                           : SMAIntArrayAccess(*KSTEP == 2 ? 13 : 67);

      // if (ia != ia2)
      //   mooseError("Mismatching int pointers ", ia, " != ", ia2);

      Moose::out << *LOP << "lop " << fasum << ' '
                 << (threaded ? SMALocalFloatArraySize(7) : SMAFloatArraySize(7)) << ' ' << iasum
                 << ' ' << (threaded ? SMALocalIntArraySize(67) : SMAIntArraySize(67)) << '\n';

      break;
    }

    // beginning of the timestep
    case 1:
    {
      // AbaqusUtils::smaDebug();

      Real * fa = threaded ? SMALocalFloatArrayAccess(7) : SMAFloatArrayAccess(7);
      for (unsigned int i = 0; i < (threaded ? SMALocalFloatArraySize(7) : SMAFloatArraySize(7));
           ++i)
        fa[i] += i + *KINC;

      int * ia = threaded ? SMALocalIntArrayAccess(67) : SMAIntArrayAccess(67);
      for (unsigned int i = 0; i < (threaded ? SMALocalIntArraySize(67) : SMAIntArraySize(67)); ++i)
        ia[i] += i + *KINC;
      break;
    }

    // end of the simulation
    case 3:
    {
      Real * fa = threaded ? SMALocalFloatArrayAccess(7) : SMAFloatArrayAccess(7);
      Real fasum = 0.0;
      for (unsigned int i = 0; i < (threaded ? SMALocalFloatArraySize(7) : SMAFloatArraySize(7));
           ++i)
        fasum += fa[i] + 0;
      if (threaded)
        SMALocalFloatArrayDelete(7);
      else
        SMAFloatArrayDelete(7);

      int iasum = 0;
      int * ia = threaded ? SMALocalIntArrayAccess(67) : SMAIntArrayAccess(67);
      for (unsigned int i = 0; i < (threaded ? SMALocalIntArraySize(67) : SMAIntArraySize(67)); ++i)
        iasum += ia[i];

      if (threaded)
        SMALocalIntArrayDelete(67);
      else
        SMAIntArrayDelete(67);

      if (get_thread_id() == 0)
        Moose::out << *LOP << "lop " << fasum << ' ' << iasum << ' ' << getnumthreads() << '\n';
      break;
    }
  }
}
