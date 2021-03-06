#ifndef CPU_H
#define CPU_H

struct cpu;

#include "common.h"

struct cpu {
  LONG a[8];
  LONG d[8];
  LONG usp;
  LONG ssp;
  LONG pc;
  WORD sr;
  uint64_t cycle;
  LONG icycle;
  int cyclecomp;
  int stopped;
  int tracedelay;
  int debug_halted;
  int prefetched_instr;
  int has_prefetched;
  int ipl1, ipl2;
};

struct cpu_state {
  char id[4];
  long size;
  char *data;
};

struct cpu_state *cpu_state_collect();
void cpu_state_restore(struct cpu_state *);

extern struct cpu *cpu;
extern int cprint_all;

#define ADD_CYCLE(x) do { cpu->icycle += x; } while(0);
#define MAX_CYCLE 8012800

#define MSKT 0x8000
#define MSKS 0x2000
#define MSKX 0x10
#define MSKN 0x8
#define MSKZ 0x4
#define MSKV 0x2
#define MSKC 0x1

#define SETX (cpu->sr |= MSKX)
#define SETN (cpu->sr |= MSKN)
#define SETZ (cpu->sr |= MSKZ)
#define SETV (cpu->sr |= MSKV)
#define SETC (cpu->sr |= MSKC)

#define CLRX (cpu->sr &= ~MSKX)
#define CLRN (cpu->sr &= ~MSKN)
#define CLRZ (cpu->sr &= ~MSKZ)
#define CLRV (cpu->sr &= ~MSKV)
#define CLRC (cpu->sr &= ~MSKC)

#define CHKS (cpu->sr & MSKS)
#define CHKT (cpu->sr & MSKT)
#define CHKX (cpu->sr & MSKX)
#define CHKN (cpu->sr & MSKN)
#define CHKZ (cpu->sr & MSKZ)
#define CHKV (cpu->sr & MSKV)
#define CHKC (cpu->sr & MSKC)

#define IPL ((cpu->sr & 0x700)>>8)
#define IPL_HBL 2
#define IPL_VBL 4
#define IPL_MFP 6
#define IPL_NO_AUTOVECTOR -1
#define IPL_EXCEPTION_VECTOR_OFFSET 24

#define VEC_BUSERR  2
#define VEC_ADDRERR 3
#define VEC_ILLEGAL 4
#define VEC_ZERODIV 5
#define VEC_CHK     6
#define VEC_TRAPV   7
#define VEC_PRIV    8
#define VEC_TRACE   9
#define VEC_LINEA  10
#define VEC_LINEF  11
#define VEC_SPUR   24

#define CPU_STACKFRAME_DATA  0x08

#define CPU_BUSERR_READ  0x10
#define CPU_BUSERR_WRITE 0
#define CPU_BUSERR_INSTR 0
#define CPU_BUSERR_DATA  0x08

#define CPU_ADDRERR_READ  0x10
#define CPU_ADDRERR_WRITE 0
#define CPU_ADDRERR_INSTR 0
#define CPU_ADDRERR_DATA  0x08

#define CPU_USE_CURRENT_PC 0
#define CPU_USE_LAST_PC    1

#define CPU_OK 0
#define CPU_BREAKPOINT 1
#define CPU_WATCHPOINT 2

#define CPU_RUN 0
#define CPU_TRACE 1
#define CPU_DEBUG_RUN -1

#define CPU_DO_INTS 0
#define CPU_NO_INTS 1

#define CPU_WATCH_EQ 0
#define CPU_WATCH_NE 1
#define CPU_WATCH_GT 2
#define CPU_WATCH_GE 3
#define CPU_WATCH_LT 4
#define CPU_WATCH_LE 5

void cpu_init();
void cpu_halt_for_debug();
void cpu_enter_debugger();
void cpu_prefetch();
void cpu_clear_prefetch();
int cpu_step_instr(int);
void cpu_print_status(int);
void cpu_do_cycle(LONG);
void cpu_check_for_pending_interrupts();
void cpu_set_interrupt(int, int);
void cpu_set_exception(int);
void cpu_clr_exception(int);
int cpu_full_stacked_exception_pending();
void cpu_set_bus_error(int, LONG);
void cpu_set_address_error(int, LONG);
void cpu_add_debugpoint(LONG);
void cpu_set_sr(WORD);
int cpu_find_breakpoint_lowest_cnt(LONG);
void cpu_set_watchpoint(char *, int);
void cpu_set_breakpoint(LONG, int);
void cpu_print_breakpoints();
int cpu_unset_breakpoint(LONG);
int cpu_run(int);
void cpu_reset(void);
void cpu_add_extra_cycles(int);
void cpu_ipl1(void);
void cpu_ipl2(void);

void cprint_set_label(LONG, char *);
char *cprint_find_label(LONG);
struct cprint *cprint_instr(LONG);
void cprint_save_labels(char *);
void cprint_load_labels(char *);
int cprint_label_exists(char *);
LONG cprint_label_addr(char *);

static void cpu_set_flags_general(struct cpu *cpu, int mask,
				  int rm, int r)
{
  if(mask&MSKN) { if(rm) SETN; else CLRN; }
  if(mask&MSKX) { if(CHKC) SETX; else CLRX; }
  if(mask&MSKZ) { if(!r) SETZ; else CLRZ; }
}

/* SUB, SUBI, SUBQ */
static void cpu_set_flags_sub(struct cpu *cpu, int sm, int dm, int rm, int r)
{
  if((!sm && dm && !rm) || (sm && !dm && rm)) SETV; else CLRV;
  if((sm && !dm) || (rm && !dm) || (sm && rm)) SETC; else CLRC;
  cpu_set_flags_general(cpu, MSKN | MSKZ | MSKX, rm, r);
}

/* CAS, CAS2, CMP, CMPI, CMPM */
static void cpu_set_flags_cmp(struct cpu *cpu, int sm, int dm, int rm, int r)
{
  if((!sm && dm && !rm) || (sm && !dm && rm)) SETV; else CLRV;
  if((sm && !dm) || (rm && !dm) || (sm && rm)) SETC; else CLRC;
  cpu_set_flags_general(cpu, MSKN | MSKZ, rm, r);
}

/* ADD, ADDI, ADDQ */
static void cpu_set_flags_add(struct cpu *cpu, int sm, int dm, int rm, int r)
{
  if((sm && dm && !rm) || (!sm && !dm && rm)) SETV; else CLRV;
  if((sm && dm) || (!rm && dm) || (sm && !rm)) SETC; else CLRC;
  cpu_set_flags_general(cpu, MSKN | MSKZ | MSKX, rm, r);
}

/* MOVE, MOVEQ, AND, ANDI, MULU, OR, ORI, TST */
static void cpu_set_flags_move(struct cpu *cpu, int rm, int r)
{
  if(rm) SETN; else CLRN;
  if(!r) SETZ; else CLRZ;
  CLRV;
  CLRC;
}

/* NEG */
static void cpu_set_flags_neg(struct cpu *cpu, int dm, int rm, int r)
{
  if(dm && rm) SETV; else CLRV;
  if(dm || rm) SETC; else CLRC;
  cpu_set_flags_general(cpu, MSKX | MSKN | MSKZ, rm, r);
}

/* DIVU */
static void cpu_set_flags_divu(struct cpu *cpu, int rm, int r, int v)
{
  if(rm) SETN; else CLRN;
  if(!r) SETZ; else CLRZ;
  if(v) SETV; else CLRV;
  CLRC;
}

/* DIVS */
static void cpu_set_flags_divs(struct cpu *cpu, int rm, int r)
{
  if(rm) SETN; else CLRN;
  if(!r) SETZ; else CLRZ;
  if(((r&0xffff8000) != 0) && ((r&0xffff8000) != 0xffff8000)) SETV; else CLRV;
  CLRC;
}

/* CLR */
static void cpu_set_flags_clr(struct cpu *cpu)
{
  CLRN;
  CLRV;
  CLRC;
  SETZ;
}

/* LSR */
static void cpu_set_flags_lsr(struct cpu *cpu, int rm, int r, int cnt, int hb)
{
  CLRV;
  if(cnt) {
    if(hb) SETC; else CLRC;
    cpu_set_flags_general(cpu, MSKX | MSKN | MSKZ, rm, r);
  } else {
    CLRC;
    cpu_set_flags_general(cpu, MSKN | MSKZ, rm, r);
  }
}

/* ROXL, ROXR */
static void cpu_set_flags_roxl(struct cpu *cpu, int rm, int r, int cnt, int hb)
{
  CLRV;
  if(cnt) {
    if(hb) SETC; else CLRC;
    cpu_set_flags_general(cpu, MSKX | MSKN | MSKZ, rm, r);
  } else {
    if(CHKX) SETX; else CLRX;
    cpu_set_flags_general(cpu, MSKN | MSKZ, rm, r);
  }
}

/* ROL */
static void cpu_set_flags_rol(struct cpu *cpu, int rm, int r, int cnt, int hb)
{
  CLRV;
  if(cnt) {
    if(hb) SETC; else CLRC;
    cpu_set_flags_general(cpu, MSKN | MSKZ, rm, r);
  } else {
    CLRC;
    cpu_set_flags_general(cpu, MSKN | MSKZ, rm, r);
  }
}

/* ASL */
static void cpu_set_flags_asl(struct cpu *cpu, int rm, int r, int cnt,
			      int hb, int vb)
{
  if(cnt) {
    if(hb) SETC; else CLRC;
    if(vb) SETV; else CLRV;
    cpu_set_flags_general(cpu, MSKX | MSKN | MSKZ, rm, r);
  } else {
    CLRV;
    CLRC;
    cpu_set_flags_general(cpu, MSKN | MSKZ, rm, r);
  }
}

/* ADDX */
static void cpu_set_flags_addx(struct cpu *cpu, int sm, int dm, int rm, int r)
{
  if(r) CLRZ;
  if((sm && dm && !rm) || (!sm && !dm && rm)) SETV; else CLRV;
  if((sm && dm) || (!rm && dm) || (sm && !rm)) SETC; else CLRC;
  cpu_set_flags_general(cpu, MSKN | MSKX, rm, r);
}

/* SUBX */
static void cpu_set_flags_subx(struct cpu *cpu, int sm, int dm, int rm, int r)
{
  if(r) CLRZ;
  if((!sm && dm && !rm) || (sm && !dm && rm)) SETV; else CLRV;
  if((sm && !dm) || (rm && !dm) || (sm && rm)) SETC; else CLRC;
  cpu_set_flags_general(cpu, MSKN | MSKX, rm, r);
}

/* NEGX */
static void cpu_set_flags_negx(struct cpu *cpu, int dm, int rm, int r)
{
  if(dm && rm) SETV; else CLRV;
  if(dm || rm) SETC; else CLRC;
  if(r) CLRZ;
  cpu_set_flags_general(cpu, MSKN | MSKX, rm, r);
}

#endif
