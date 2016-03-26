auto R65816::instruction() -> void {
  #define opA(  n, o      ) case n: return op_##o();
  #define opAII(n, o, i, j) case n: return op_##o(i, j);
  #define opE(  n, o      ) case n: return regs.e ? op_##o##_e() : op_##o##_n();
  #define opEI( n, o, i   ) case n: return regs.e ? op_##o##_e(i) : op_##o##_n(i);
  #define opEII(n, o, i, j) case n: return regs.e ? op_##o##_e(i) : op_##o##_n(j);
  #define opM(  n, o      ) case n: return regs.p.m ? op_##o##_b() : op_##o##_w();
  #define opMF( n, o, f   ) case n: return regs.p.m ? op_##o##_b(&R65816::op_##f##_b) : op_##o##_w(&R65816::op_##f##_w);
  #define opMFI(n, o, f, i) case n: return regs.p.m ? op_##o##_b(&R65816::op_##f##_b, i) : op_##o##_w(&R65816::op_##f##_w, i);
  #define opMI( n, o, i   ) case n: return regs.p.m ? op_##o##_b(i) : op_##o##_w(i);
  #define opMII(n, o, i, j) case n: return regs.p.m ? op_##o##_b(i, j) : op_##o##_w(i, j);
  #define opX(  n, o)       case n: return regs.p.x ? op_##o##_b() : op_##o##_w();
  #define opXF( n, o, f   ) case n: return regs.p.x ? op_##o##_b(&R65816::op_##f##_b) : op_##o##_w(&R65816::op_##f##_w);
  #define opXFI(n, o, f, i) case n: return regs.p.x ? op_##o##_b(&R65816::op_##f##_b, i) : op_##o##_w(&R65816::op_##f##_w, i);
  #define opXI( n, o, i   ) case n: return regs.p.x ? op_##o##_b(i) : op_##o##_w(i);
  #define opXII(n, o, i, j) case n: return regs.p.x ? op_##o##_b(i, j) : op_##o##_w(i, j);

  switch(readpc()) {
  opEII(0x00, interrupt, 0xfffe, 0xffe6)
  opMF (0x01, read_idpx, ora)
  opEII(0x02, interrupt, 0xfff4, 0xffe4)
  opMF (0x03, read_sr, ora)
  opMF (0x04, adjust_dp, tsb)
  opMF (0x05, read_dp, ora)
  opMF (0x06, adjust_dp, asl)
  opMF (0x07, read_ildp, ora)
  opA  (0x08, php)
  opMF (0x09, read_const, ora)
  opM  (0x0a, asl_imm)
  opE  (0x0b, phd)
  opMF (0x0c, adjust_addr, tsb)
  opMF (0x0d, read_addr, ora)
  opMF (0x0e, adjust_addr, asl)
  opMF (0x0f, read_long, ora)
  opAII(0x10, branch, regs.p.n, 0)
  opMF (0x11, read_idpy, ora)
  opMF (0x12, read_idp, ora)
  opMF (0x13, read_isry, ora)
  opMF (0x14, adjust_dp, trb)
  opMFI(0x15, read_dpr, ora, regs.x)
  opMF (0x16, adjust_dpx, asl)
  opMF (0x17, read_ildpy, ora)
  opAII(0x18, flag, regs.p.c, 0)
  opMF (0x19, read_addry, ora)
  opMII(0x1a, adjust_imm, regs.a, +1)
  opE  (0x1b, tcs)
  opMF (0x1c, adjust_addr, trb)
  opMF (0x1d, read_addrx, ora)
  opMF (0x1e, adjust_addrx, asl)
  opMF (0x1f, read_longx, ora)
  opA  (0x20, jsr_addr)
  opMF (0x21, read_idpx, and)
  opE  (0x22, jsr_long)
  opMF (0x23, read_sr, and)
  opMF (0x24, read_dp, bit)
  opMF (0x25, read_dp, and)
  opMF (0x26, adjust_dp, rol)
  opMF (0x27, read_ildp, and)
  opE  (0x28, plp)
  opMF (0x29, read_const, and)
  opM  (0x2a, rol_imm)
  opE  (0x2b, pld)
  opMF (0x2c, read_addr, bit)
  opMF (0x2d, read_addr, and)
  opMF (0x2e, adjust_addr, rol)
  opMF (0x2f, read_long, and)
  opAII(0x30, branch, regs.p.n, 1)
  opMF (0x31, read_idpy, and)
  opMF (0x32, read_idp, and)
  opMF (0x33, read_isry, and)
  opMFI(0x34, read_dpr, bit, regs.x)
  opMFI(0x35, read_dpr, and, regs.x)
  opMF (0x36, adjust_dpx, rol)
  opMF (0x37, read_ildpy, and)
  opAII(0x38, flag, regs.p.c, 1)
  opMF (0x39, read_addry, and)
  opMII(0x3a, adjust_imm, regs.a, -1)
  opAII(0x3b, transfer_w, regs.s, regs.a)
  opMF (0x3c, read_addrx, bit)
  opMF (0x3d, read_addrx, and)
  opMF (0x3e, adjust_addrx, rol)
  opMF (0x3f, read_longx, and)
  opE  (0x40, rti)
  opMF (0x41, read_idpx, eor)
  opA  (0x42, wdm)
  opMF (0x43, read_sr, eor)
  opXI (0x44, move, -1)
  opMF (0x45, read_dp, eor)
  opMF (0x46, adjust_dp, lsr)
  opMF (0x47, read_ildp, eor)
  opMI (0x48, push, regs.a)
  opMF (0x49, read_const, eor)
  opM  (0x4a, lsr_imm)
  opA  (0x4b, phk)
  opA  (0x4c, jmp_addr)
  opMF (0x4d, read_addr, eor)
  opMF (0x4e, adjust_addr, lsr)
  opMF (0x4f, read_long, eor)
  opAII(0x50, branch, regs.p.v, 0)
  opMF (0x51, read_idpy, eor)
  opMF (0x52, read_idp, eor)
  opMF (0x53, read_isry, eor)
  opXI (0x54, move, +1)
  opMFI(0x55, read_dpr, eor, regs.x)
  opMF (0x56, adjust_dpx, lsr)
  opMF (0x57, read_ildpy, eor)
  opAII(0x58, flag, regs.p.i, 0)
  opMF (0x59, read_addry, eor)
  opXI (0x5a, push, regs.y)
  opAII(0x5b, transfer_w, regs.a, regs.d)
  opA  (0x5c, jmp_long)
  opMF (0x5d, read_addrx, eor)
  opMF (0x5e, adjust_addrx, lsr)
  opMF (0x5f, read_longx, eor)
  opA  (0x60, rts)
  opMF (0x61, read_idpx, adc)
  opE  (0x62, per)
  opMF (0x63, read_sr, adc)
  opMI (0x64, write_dp, regs.z)
  opMF (0x65, read_dp, adc)
  opMF (0x66, adjust_dp, ror)
  opMF (0x67, read_ildp, adc)
  opMI (0x68, pull, regs.a)
  opMF (0x69, read_const, adc)
  opM  (0x6a, ror_imm)
  opE  (0x6b, rtl)
  opA  (0x6c, jmp_iaddr)
  opMF (0x6d, read_addr, adc)
  opMF (0x6e, adjust_addr, ror)
  opMF (0x6f, read_long, adc)
  opAII(0x70, branch, regs.p.v, 1)
  opMF (0x71, read_idpy, adc)
  opMF (0x72, read_idp, adc)
  opMF (0x73, read_isry, adc)
  opMII(0x74, write_dpr, regs.z, regs.x)
  opMFI(0x75, read_dpr, adc, regs.x)
  opMF (0x76, adjust_dpx, ror)
  opMF (0x77, read_ildpy, adc)
  opAII(0x78, flag, regs.p.i, 1)
  opMF (0x79, read_addry, adc)
  opXI (0x7a, pull, regs.y)
  opAII(0x7b, transfer_w, regs.d, regs.a)
  opA  (0x7c, jmp_iaddrx)
  opMF (0x7d, read_addrx, adc)
  opMF (0x7e, adjust_addrx, ror)
  opMF (0x7f, read_longx, adc)
  opA  (0x80, bra)
  opM  (0x81, sta_idpx)
  opA  (0x82, brl)
  opM  (0x83, sta_sr)
  opXI (0x84, write_dp, regs.y)
  opMI (0x85, write_dp, regs.a)
  opXI (0x86, write_dp, regs.x)
  opM  (0x87, sta_ildp)
  opXII(0x88, adjust_imm, regs.y, -1)
  opM  (0x89, read_bit_const)
  opMII(0x8a, transfer, regs.x, regs.a)
  opA  (0x8b, phb)
  opXI (0x8c, write_addr, regs.y)
  opMI (0x8d, write_addr, regs.a)
  opXI (0x8e, write_addr, regs.x)
  opMI (0x8f, write_longr, regs.z)
  opAII(0x90, branch, regs.p.c, 0)
  opM  (0x91, sta_idpy)
  opM  (0x92, sta_idp)
  opM  (0x93, sta_isry)
  opXII(0x94, write_dpr, regs.y, regs.x)
  opMII(0x95, write_dpr, regs.a, regs.x)
  opXII(0x96, write_dpr, regs.x, regs.y)
  opM  (0x97, sta_ildpy)
  opMII(0x98, transfer, regs.y, regs.a)
  opMII(0x99, write_addrr, regs.a, regs.y)
  opE  (0x9a, txs)
  opXII(0x9b, transfer, regs.x, regs.y)
  opMI (0x9c, write_addr, regs.z)
  opMII(0x9d, write_addrr, regs.a, regs.x)
  opMII(0x9e, write_addrr, regs.z, regs.x)
  opMI (0x9f, write_longr, regs.x)
  opXF (0xa0, read_const, ldy)
  opMF (0xa1, read_idpx, lda)
  opXF (0xa2, read_const, ldx)
  opMF (0xa3, read_sr, lda)
  opXF (0xa4, read_dp, ldy)
  opMF (0xa5, read_dp, lda)
  opXF (0xa6, read_dp, ldx)
  opMF (0xa7, read_ildp, lda)
  opXII(0xa8, transfer, regs.a, regs.y)
  opMF (0xa9, read_const, lda)
  opXII(0xaa, transfer, regs.a, regs.x)
  opA  (0xab, plb)
  opXF (0xac, read_addr, ldy)
  opMF (0xad, read_addr, lda)
  opXF (0xae, read_addr, ldx)
  opMF (0xaf, read_long, lda)
  opAII(0xb0, branch, regs.p.c, 1)
  opMF (0xb1, read_idpy, lda)
  opMF (0xb2, read_idp, lda)
  opMF (0xb3, read_isry, lda)
  opXFI(0xb4, read_dpr, ldy, regs.x)
  opMFI(0xb5, read_dpr, lda, regs.x)
  opXFI(0xb6, read_dpr, ldx, regs.y)
  opMF (0xb7, read_ildpy, lda)
  opAII(0xb8, flag, regs.p.v, 0)
  opMF (0xb9, read_addry, lda)
  opX  (0xba, tsx)
  opXII(0xbb, transfer, regs.y, regs.x)
  opXF (0xbc, read_addrx, ldy)
  opMF (0xbd, read_addrx, lda)
  opXF (0xbe, read_addry, ldx)
  opMF (0xbf, read_longx, lda)
  opXF (0xc0, read_const, cpy)
  opMF (0xc1, read_idpx, cmp)
  opEI (0xc2, pflag, 0)
  opMF (0xc3, read_sr, cmp)
  opXF (0xc4, read_dp, cpy)
  opMF (0xc5, read_dp, cmp)
  opMF (0xc6, adjust_dp, dec)
  opMF (0xc7, read_ildp, cmp)
  opXII(0xc8, adjust_imm, regs.y, +1)
  opMF (0xc9, read_const, cmp)
  opXII(0xca, adjust_imm, regs.x, -1)
  opA  (0xcb, wai)
  opXF (0xcc, read_addr, cpy)
  opMF (0xcd, read_addr, cmp)
  opMF (0xce, adjust_addr, dec)
  opMF (0xcf, read_long, cmp)
  opAII(0xd0, branch, regs.p.z, 0)
  opMF (0xd1, read_idpy, cmp)
  opMF (0xd2, read_idp, cmp)
  opMF (0xd3, read_isry, cmp)
  opE  (0xd4, pei)
  opMFI(0xd5, read_dpr, cmp, regs.x)
  opMF (0xd6, adjust_dpx, dec)
  opMF (0xd7, read_ildpy, cmp)
  opAII(0xd8, flag, regs.p.d, 0)
  opMF (0xd9, read_addry, cmp)
  opXI (0xda, push, regs.x)
  opA  (0xdb, stp)
  opA  (0xdc, jmp_iladdr)
  opMF (0xdd, read_addrx, cmp)
  opMF (0xde, adjust_addrx, dec)
  opMF (0xdf, read_longx, cmp)
  opXF (0xe0, read_const, cpx)
  opMF (0xe1, read_idpx, sbc)
  opEI (0xe2, pflag, 1)
  opMF (0xe3, read_sr, sbc)
  opXF (0xe4, read_dp, cpx)
  opMF (0xe5, read_dp, sbc)
  opMF (0xe6, adjust_dp, inc)
  opMF (0xe7, read_ildp, sbc)
  opXII(0xe8, adjust_imm, regs.x, +1)
  opMF (0xe9, read_const, sbc)
  opA  (0xea, nop)
  opA  (0xeb, xba)
  opXF (0xec, read_addr, cpx)
  opMF (0xed, read_addr, sbc)
  opMF (0xee, adjust_addr, inc)
  opMF (0xef, read_long, sbc)
  opAII(0xf0, branch, regs.p.z, 1)
  opMF (0xf1, read_idpy, sbc)
  opMF (0xf2, read_idp, sbc)
  opMF (0xf3, read_isry, sbc)
  opE  (0xf4, pea)
  opMFI(0xf5, read_dpr, sbc, regs.x)
  opMF (0xf6, adjust_dpx, inc)
  opMF (0xf7, read_ildpy, sbc)
  opAII(0xf8, flag, regs.p.d, 1)
  opMF (0xf9, read_addry, sbc)
  opXI (0xfa, pull, regs.x)
  opA  (0xfb, xce)
  opE  (0xfc, jsr_iaddrx)
  opMF (0xfd, read_addrx, sbc)
  opMF (0xfe, adjust_addrx, inc)
  opMF (0xff, read_longx, sbc)
  }

  #undef opA
  #undef opAII
  #undef opE
  #undef opEI
  #undef opEII
  #undef opM
  #undef opMF
  #undef opMFI
  #undef opMI
  #undef opMII
  #undef opX
  #undef opXF
  #undef opXFI
  #undef opXI
  #undef opXII
}
