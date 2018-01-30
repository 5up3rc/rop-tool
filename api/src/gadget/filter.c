/************************************************************************/
/* rop-tool - A Return Oriented Programming and binary exploitation     */
/*            tool                                                      */
/*                                                                      */
/* Copyright 2013-2015, -TOSH-                                          */
/* File coded by -TOSH-                                                 */
/*                                                                      */
/* This file is part of rop-tool.                                       */
/*                                                                      */
/* rop-tool is free software: you can redistribute it and/or modify     */
/* it under the terms of the GNU General Public License as published by */
/* the Free Software Foundation, either version 3 of the License, or    */
/* (at your option) any later version.                                  */
/*                                                                      */
/* rop-tool is distributed in the hope that it will be useful,          */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU General Public License for more details.                         */
/*                                                                      */
/* You should have received a copy of the GNU General Public License    */
/* along with rop-tool.  If not, see <http://www.gnu.org/licenses/>     */
/************************************************************************/
#include "disassemble.h"
#include "binfmt.h"


/* =========================================================================
   This file implement functions for filter and matching gadgets
   ======================================================================= */

/*
 * %X  : hexad�cimal value
 * %Q : qword (64bits) register (r8, r9, r10, r11, r12, r13, r14, r15,rax, rbx, rcx, rdx, rsi, rdi, rsp);
 * %D : dword (32bits) register (eax, ebx, ecx, edx, esi, edi, esp, ebp)
 * %W : word (16bits) register (ax, bx, cx, dx, si, di)
 * %B : byte (8bits) register (al, bl, cl, dl)
 * %% : '%' char
 */

static const char *intel_x86_filters[] = {
  "pop %D",
  "popa",

  "push %D",
  "pusha",

  "add %D, dword ptr [%X]",
  "add %D, dword ptr [%D + %X]",
  "add %D, dword ptr [%D - %X]",
  "add %D, dword ptr [%D]",
  "add %D, %X",
  "add %D, %D",
  "add dword ptr [%D], %D",
  "add dword ptr [%D + %X], %D",
  "add dword ptr [%D - %X], %D",

  "int %X",
  "call %D",
  "call dword ptr [%D]",
  "jmp dword ptr [%D]",
  "jmp %D",

  "mov %D, %D",
  "mov dword ptr [%D + %X], %D",
  "mov dword ptr [%D - %X], %D",
  "mov dword ptr [%D], %D",
  "mov %D, dword ptr [%D]",
  "mov %D, dword ptr [%D + %X]",
  "mov %D, dword ptr [%D - %X]",
  "mov %b, %b",

  "add byte ptr [%D], %B",
  "add byte ptr [%D + %X], %B",
  "add byte ptr [%D - %X], %B",

  "xchg %D, %D",

  "inc %D",
  "inc %W",
  "inc %B",

  "dec %D",
  "dec %W",
  "dec %B",

  "leave ",
  "ret ",
  NULL
};

static const char *intel_x86_64_filters[] = {
  "pop %D",
  "pop %Q",
  "popa",

  "push %D",
  "push %Q",
  "pusha",

  "add %D, dword ptr [%X]",
  "add %D, dword ptr [%D + %X]",
  "add %D, dword ptr [%D - %X]",
  "add %D, dword ptr [%D]",
  "add %D, %X",
  "add %D, %D",
  "add dword ptr [%D], %D",
  "add dword ptr [%D + %X], %D",
  "add dword ptr [%D - %X], %D",

  "add %Q, dword ptr [%X]",
  "add %Q, qword ptr [%Q+%X]",
  "add %Q, qword ptr [%Q - %X]",
  "add %Q, qword ptr [%Q]",
  "add %Q, %X",
  "add %Q, %Q",
  "add qword ptr [%Q], %Q",
  "add qword ptr [%Q + %X], %Q",
  "add qword ptr [%Q - %X], %Q",

  "int %X",
  "call %D",
  "call %Q",
  "call dword ptr [%D]",
  "call qword ptr [%Q]",
  "jmp dword ptr [%D]",
  "jmp qword ptr [%Q]",
  "jmp %D",
  "jmp %Q",

  "mov %D, %D",
  "mov dword ptr [%D + %X], %D",
  "mov dword ptr [%D - %X], %D",
  "mov dword ptr [%D], %D",
  "mov %D, dword ptr [%D]",
  "mov %D, dword ptr [%D + %X]",
  "mov %D, dword ptr [%D - %X]",

  "mov %Q, %Q",
  "mov qword ptr [%Q + %X], %Q",
  "mov qword ptr [%Q - %X], %Q",
  "mov qword ptr [%Q], %Q",
  "mov %Q, qword ptr [%Q]",
  "mov %Q, qword ptr [%Q + %X]",
  "mov %Q, qword ptr [%Q - %X]",

  "mov %B, %B",

  "add byte ptr [%D], %B",
  "add byte ptr [%D + %X], %B",
  "add byte ptr [%D - %X], %B",

  "add byte ptr [%Q], %B",
  "add byte ptr [%Q + %X], %B",
  "add byte ptr [%Q - %X], %B",

  "xchg %D, %D",
  "xchg %Q, %Q",

  "inc %Q",
  "inc %D",
  "inc %W",
  "inc %B",

  "dec %Q",
  "dec %D",
  "dec %W",
  "dec %B",

  "syscall ",
  "leave ",
  "ret ",
  NULL
};

static const char *att_x86_filters[] = {
  "popl %%%D",
  "popa",

  "pushl %%%D",
  "pusha",

  "addl (%%%D), %%%D",
  "addl %%%D, (%%%D)",
  "addl %%%D, $%X",
  "addl %%%D, %%%D",
  "addl %%%D, (%%%D)",

  "addl %%%D, %X(%%%D)",
  "addb %%%B, %X(%%%D)",
  "addb %%%B, (%%%D)",
  "addl $%X, %%%D",
  "addl %X, %%%D",
  "addl %X(%%%D), %%%D",

  "int $%X",
  "calll *(%%%D)",
  "calll *%%%D",
  "jmpl *%%%D",
  "jmpl *(%%%D)",

  "movl %%%D, %%%D",
  "movl %%%D, (%%%D)",
  "movl (%%%D), %%%D",
  "movb %%%B, %%%B",
  "movl %X(%%%D), %%%D",
  "movl %%%D, %X(%%%D)",

  "xchgl %%%D, %%%D",


  "incl %%%D",
  "incb %%%B",

  "decl %%%D",
  "decb %%%B",

  "leave ",
  "retl ",
  NULL
};

static const char *att_x86_64_filters[] = {
  "popl %%%D",
  "popq %%%Q",
  "popa",

  "pushl %%%D",
  "pushq %%%Q",
  "pusha",

  "addl (%%%D), %%%D",
  "addl %%%D, (%%%D)",
  "addl %%%D, $%X",
  "addl %%%D, %%%D",
  "addl %%%D, (%%%D)",

  "addl %%%D, %X(%%%D)",
  "addl $%X, %%%D",
  "addl %X, %%%D",
  "addl %X(%%%D), %%%D",
  "addb %%%B, %X(%%%D)",
  "addb %%%B, (%%%D)",
  "addb %%%B, %X(%%%Q)",
  "addb %%%B, (%%%Q)",
  "addq $%X, %%%Q",

  "int $%X",
  "calll *(%%%D)",
  "calll *%%%D",
  "callq *%%%Q",
  "callq *(%%%Q)",
  "jmpl *%%%D",
  "jmpl *(%%%D)",
  "jmpq *%%%Q",
  "jmpq *(%%%Q)",

  "movl %%%D, %%%D",
  "movl %%%D, (%%%D)",
  "movl (%%%D), %%%D",
  "movl %X(%%%D), %%%D",
  "movl %%%D, %X(%%%D)",
  "movb %%%B, %%%B",
  "movq %X(%%%Q), %%%Q",
  "movq %X(%%%Q), %%%Q",
  "movq %%%Q, %%%Q",
  "movq %%%Q, (%%%Q)",
  "movq (%%%Q), %%%Q",
  "movq %%%Q, %X(%%%Q)",

  "xchgl %%%D, %%%D",
  "xchgq %%%Q, %%%Q",

  "incl %%%D",
  "incb %%%B",

  "decl %%%D",
  "decb %%%B",

  "leave ",
  "retl ",
  "retq ",
  NULL
};


/* Return true if the instruction match the filter */
int r_gadget_filter_strncmp(const char *gadget, const char *filter, int len) {
  const char *p1 = filter;
  const char *p2 = gadget;
  int i;

  i = 0;
  while((len == 0 || i < len)
        && *p1 != '\0' && p2[i] != '\0') {
    if(*p1 == '%') {

      p1++;
      if(*p1 == '%') {
        if(p2[i] != '%')
          break;
      }
      if(*p1 == 'X') {
        if(p2[i] == '-')
          i++;
        if(p2[i] != '0' && p2[i+1] != 'x')
          break;
  i += 2;
  while(isxdigit(p2[i]))
    i++;
  i--;
      }
      if(*p1 == 'Q') {
        if(strncmp("r8", p2+i,2)   &&
           strncmp("r9", p2+i,2)   &&
           strncmp("r10", p2+i,3)  &&
           strncmp("r11", p2+i,3)  &&
           strncmp("r12", p2+i,3)  &&
           strncmp("r13", p2+i,3)  &&
           strncmp("r14", p2+i,3)  &&
           strncmp("r15", p2+i,3)  &&
           strncmp("rax", p2+i, 3) &&
           strncmp("rbx", p2+i, 3) &&
           strncmp("rcx", p2+i, 3) &&
           strncmp("rdx", p2+i, 3) &&
           strncmp("rsp", p2+i, 3) &&
           strncmp("rbp", p2+i, 3) &&
           strncmp("rsi", p2+i, 3) &&
           strncmp("rdi", p2+i, 3))
          break;
        i += 2;
      }
      if(*p1 == 'D') {
        if(strncmp("eax", p2+i, 3) &&
           strncmp("ebx", p2+i, 3) &&
           strncmp("ecx", p2+i, 3) &&
           strncmp("edx", p2+i, 3) &&
           strncmp("esp", p2+i, 3) &&
           strncmp("ebp", p2+i, 3) &&
           strncmp("esi", p2+i, 3) &&
           strncmp("edi", p2+i, 3))
          break;
  i += 2;
      }
      if(*p1 == 'W') {
        if(strncmp("ax", p2+i, 2) &&
           strncmp("bx", p2+i, 2) &&
           strncmp("cx", p2+i, 2) &&
           strncmp("dx", p2+i, 2) &&
           strncmp("di", p2+i, 2) &&
           strncmp("si", p2+i, 2))
          break;
        i++;
      }

      if(*p1 == 'B') {
        if(strncmp("al", p2+i, 2) &&
           strncmp("bl", p2+i, 2) &&
           strncmp("cl", p2+i, 2) &&
           strncmp("dl", p2+i, 2))
          break;
        i++;

      }
    } else {
      if(*p1 != p2[i])
        break;
    }
    p1++;
    i++;
  }
  if(*p1 == '\0' && (i == len || p2[i] == '\0'))
    return 1;

  return 0;
}

/* Return true if the gadget match filters */
int r_gadget_is_filter(const char *gadget, r_binfmt_arch_e arch, r_disa_flavor_e flavor) {
  const char **p_filters;
  int i;
  const char *p;
  int match;


  /* Check wich filter to use */
  if(flavor == R_DISA_FLAVOR_INTEL && arch == R_BINFMT_ARCH_X86) {
    p_filters = intel_x86_filters;
  }  else if(flavor == R_DISA_FLAVOR_ATT && arch == R_BINFMT_ARCH_X86) {
    p_filters = att_x86_filters;
  } else if(flavor == R_DISA_FLAVOR_INTEL && arch == R_BINFMT_ARCH_X86_64) {
    p_filters = intel_x86_64_filters;
  } else if(flavor == R_DISA_FLAVOR_ATT && arch == R_BINFMT_ARCH_X86_64) {
    p_filters = att_x86_64_filters;
  } else {
    /* No filter available for this flavor/architecture : don't filter gadget */
    return 1;
  }

  while((p = strchr(gadget, ';')) != NULL) {
    match = 0;
    for(i = 0; p_filters[i] != NULL; i++) {
      if(r_gadget_filter_strncmp(gadget, p_filters[i], p-gadget)) {
  match = 1;
      }
    }

    if(!match)
      return 0;

    gadget = p+2;
  }
  return 1;
}
