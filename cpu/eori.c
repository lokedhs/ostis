#include "common.h"
#include "cpu.h"
#include "cprint.h"
#include "mmu.h"
#include "ea.h"

static void eori_b(struct cpu *cpu, WORD op)
{
  BYTE s,d,r;

  if(op&0x38) {
    ADD_CYCLE(12);
  } else {
    ADD_CYCLE(8);
  }
  s = bus_read_word(cpu->pc)&0xff;
  cpu->pc += 2;
  d = ea_read_byte(cpu, op&0x3f, 1);
  r = s^d;
  ea_set_prefetch_before_write();
  ea_write_byte(cpu, op&0x3f, r);
  cpu_set_flags_move(cpu, r&0x80, r);
}

static void eori_w(struct cpu *cpu, WORD op)
{
  WORD s,d,r;

  if(op&0x38) {
    ADD_CYCLE(12);
  } else {
    ADD_CYCLE(8);
  }
  s = bus_read_word(cpu->pc);
  cpu->pc += 2;
  d = ea_read_word(cpu, op&0x3f, 1);
  r = s^d;
  ea_set_prefetch_before_write();
  ea_write_word(cpu, op&0x3f, r);
  cpu_set_flags_move(cpu, r&0x8000, r);
}

static void eori_l(struct cpu *cpu, WORD op)
{
  LONG s,d,r;

  if(op&0x38) {
    ADD_CYCLE(20);
  } else {
    ADD_CYCLE(16);
  }
  s = bus_read_long(cpu->pc);
  cpu->pc += 4;
  d = ea_read_long(cpu, op&0x3f, 1);
  r = s^d;
  ea_set_prefetch_before_write();
  ea_write_long(cpu, op&0x3f, r);
  cpu_set_flags_move(cpu, r&0x80000000, r);
}

static void eori(struct cpu *cpu, WORD op)
{
  ENTER;

  switch((op&0xc0)>>6) {
  case 0:
    eori_b(cpu, op);
    return;
  case 1:
    eori_w(cpu, op);
    return;
  case 2:
    eori_l(cpu, op);
    return;
  }
}

static struct cprint *eori_print(LONG addr, WORD op)
{
  int s;
  struct cprint *ret;

  ret = cprint_alloc(addr);

  s = (op&0xc0)>>6;

  switch(s) {
  case 0:
    strcpy(ret->instr, "EORI.B");
    sprintf(ret->data, "#$%x,", bus_read_word_print(addr+ret->size)&0xff);
    ret->size += 2;
    break;
  case 1:
    strcpy(ret->instr, "EORI.W");
    sprintf(ret->data, "#$%x,", bus_read_word_print(addr+ret->size));
    ret->size += 2;
    break;
  case 2:
    strcpy(ret->instr, "EORI.L");
    sprintf(ret->data, "#$%x,", bus_read_long_print(addr+ret->size));
    ret->size += 4;
    break;
  }
  ea_print(ret, op&0x3f, s);

  return ret;
}

void eori_init(void *instr[], void *print[])
{
  int i;
  for(i=0;i<0xc0;i++) {
    if(ea_valid(i&0x3f, EA_INVALID_DST|EA_INVALID_A)) {
      instr[0x0a00|i] = eori;
      print[0x0a00|i] = eori_print;
    }
  }
}
