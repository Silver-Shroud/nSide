auto R6502::op_nop_immediate() {
L rd = readPC();
}

auto R6502::op_nop_absolute() {
  abs.l = readPC();
  abs.h = readPC();
L idle();
}

auto R6502::op_nop_absolute_x() {
  abs.l = readPC();
  abs.h = readPC();
  ioPage(abs.w, abs.w + r.x);
L idle();
}

auto R6502::op_nop_zero_page() {
  zp = readPC();
L readZP(zp);
}

auto R6502::op_nop_zero_page_x() {
  zp = readPC();
  readZP(zp);
L readZP(zp + r.x);
}

auto R6502::op_stp() {
  while(true) idle();
}

auto R6502::op_rmwr_absolute(fp opw, fp opr) {
  abs.l = readPC();
  abs.h = readPC();
  rd = read(abs.w);
  write(abs.w, rd);
  call(opw);
L write(abs.w, rd);
  call(opr);
}

auto R6502::op_rmwr_absolute_x(fp opw, fp opr) {
  abs.l = readPC();
  abs.h = readPC();
  ioPageAlways(abs.w, abs.w + r.x);
  rd = read(abs.w + r.x);
  write(abs.w + r.x, rd);
  call(opw);
L write(abs.w + r.x, rd);
  call(opr);
}

auto R6502::op_rmwr_absolute_y(fp opw, fp opr) {
  abs.l = readPC();
  abs.h = readPC();
  ioPageAlways(abs.w, abs.w + r.y);
  rd = read(abs.w + r.y);
  write(abs.w + r.y, rd);
  call(opw);
L write(abs.w + r.y, rd);
  call(opr);
}

auto R6502::op_rmwr_indirect_zero_page_x(fp opw, fp opr) {
  zp = readPC();
  readZP(zp);
  abs.l = readZP(zp++ + r.x);
  abs.h = readZP(zp++ + r.x);
  rd = read(abs.w);
  write(abs.w, rd);
  call(opw);
L write(abs.w, rd);
  call(opr);
}

auto R6502::op_rmwr_indirect_zero_page_y(fp opw, fp opr) {
  rd = readPC();
  abs.l = readZP(rd++);
  abs.h = readZP(rd++);
  ioPageAlways(abs.w, abs.w + r.y);
  rd = read(abs.w + r.y);
  write(abs.w + r.y, rd);
  call(opw);
L write(abs.w + r.y, rd);
  call(opr);
}

auto R6502::op_rmwr_zero_page(fp opw, fp opr) {
  zp = readPC();
  rd = readZP(zp);
  writeZP(zp, rd);
  call(opw);
L writeZP(zp, rd);
  call(opr);
}

auto R6502::op_rmwr_zero_page_x(fp opw, fp opr) {
  zp = readPC();
  readZP(zp);
  rd = readZP(zp + r.x);
  writeZP(zp + r.x, rd);
  call(opw);
L writeZP(zp + r.x, rd);
  call(opr);
}

auto R6502::op_alr_immediate() {
  op_read_immediate(&R6502::fp_and);
  fp_sra();
}

auto R6502::op_anc_immediate() {
L rd = readPC();
  fp_and();
  r.p.c = r.p.n;
}

auto R6502::op_arr_immediate() {
L rd = readPC();
  r.a &= rd;
  r.a = (r.p.c << 7) | (r.a >> 1);
  r.p.n = (r.a & 0x80);
  r.p.z = (r.a == 0);
  r.p.c = (r.a & 0x40);
  r.p.v = r.p.c ^ ((r.a >> 5) & 1);
}

auto R6502::op_axs_immediate() {
L rd = readPC();
  int result = (r.a & r.x) - rd;
  r.p.n = (result & 0x80);
  r.p.z = ((uint8)result == 0);
  r.p.c = (result >= 0);
  r.x = result;
}

auto R6502::op_dcp_absolute_y() {
  abs.l = readPC();
  abs.h = readPC();
  ioPageAlways(abs.w, abs.w + r.y);
  rd = read(abs.w + r.y);
  write(abs.w + r.y, rd);
  fp_dcp();
L write(abs.w + r.y, rd);
}

auto R6502::op_dcp_indirect_zero_page_x() {
  zp = readPC();
  readZP(zp);
  abs.l = readZP(zp++ + r.x);
  abs.h = readZP(zp++ + r.x);
  rd = read(abs.w);
  write(abs.w, rd);
  fp_dcp();
L write(abs.w, rd);
}

auto R6502::op_dcp_indirect_zero_page_y() {
  rd = readPC();
  abs.l = readZP(rd++);
  abs.h = readZP(rd++);
  ioPageAlways(abs.w, abs.w + r.y);
  rd = read(abs.w + r.y);
  write(abs.w + r.y, rd);
  fp_dcp();
L write(abs.w + r.y, rd);
}

auto R6502::op_las_absolute_y() {
  abs.l = readPC();
  abs.h = readPC();
  ioPage(abs.w, abs.w + r.y);
L rd = read(abs.w + r.y);
  r.a = r.x = r.s = rd & r.s;
}

auto R6502::op_lxa_immediate() {
L rd = readPC();
  //While other LAX opcodes decode to LDA and LDX, this one decodes
  //to LDA, LDX, and TAX, causing line noise on the data bus to interfere.
  //http://atariage.com/forums/topic/168616-lxa-stable/
  r.a = r.x = (r.a | (rand() & 0xff)) & rd;
  r.p.n = (r.a & 0x80);
  r.p.z = (r.a == 0);
}

auto R6502::op_sha_absolute_y() {
  abs.l = readPC();
  abs.h = readPC();
  ioPageAlways(abs.w, abs.w + r.y);
L write(abs.w + r.y, r.a & r.x & (abs.h + 1));
}

auto R6502::op_sha_indirect_zero_page_y() {
  rd = readPC();
  abs.l = readZP(rd++);
  abs.h = readZP(rd++);
  ioPageAlways(abs.w, abs.w + r.y);
L write(abs.w + r.y, r.a & r.x & (abs.h + 1));
}

auto R6502::op_shx_absolute_y() {
  abs.l = readPC();
  abs.h = readPC();
  ioPageAlways(abs.w, abs.w + r.y);
  if(abs.h != (abs.w + r.y) >> 8) abs.h &= r.x;
L write(abs.w + r.y, r.x & (abs.h + 1));
}

auto R6502::op_shy_absolute_x() {
  abs.l = readPC();
  abs.h = readPC();
  ioPageAlways(abs.w, abs.w + r.x);
  if(abs.h != (abs.w + r.x) >> 8) abs.h &= r.y;
L write(abs.w + r.x, r.y & (abs.h + 1));
}

auto R6502::op_tas_absolute_y() {
  abs.l = readPC();
  abs.h = readPC();
  ioPageAlways(abs.w, abs.w + r.y);
  r.s = r.a & r.x;
L write(abs.w + r.y, r.a & r.x & (abs.h + 1));
}

auto R6502::op_xaa_immediate() {
  rd = readPC();
  r.a = (r.a | (rand() & 0xff)) & r.x & rd;
}