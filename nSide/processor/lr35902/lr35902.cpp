#include <processor/processor.hpp>
#include "lr35902.hpp"

namespace Processor {

#include "instructions.cpp"
#include "disassembler.cpp"
#include "serialization.cpp"

auto LR35902::power() -> void {
  r.halt = false;
  r.stop = false;
  r.ei = false;
  r.ime = false;
}

auto LR35902::interrupt(uint16 vector) -> void {
  io();
  io();
  io();
  r.ime = 0;
  write(--r[SP], r[PC] >> 8);
  write(--r[SP], r[PC] >> 0);
  r[PC] = vector;
}

auto LR35902::instruction() -> void {
  switch(auto opcode = read(r[PC]++)) {
  case 0x00: return op_nop();
  case 0x01: return op_ld_rr_nn(BC);
  case 0x02: return op_ld_rr_a(BC);
  case 0x03: return op_inc_rr(BC);
  case 0x04: return op_inc_r(B);
  case 0x05: return op_dec_r(B);
  case 0x06: return op_ld_r_n(B);
  case 0x07: return op_rlca();
  case 0x08: return op_ld_nn_sp();
  case 0x09: return op_add_hl_rr(BC);
  case 0x0a: return op_ld_a_rr(BC);
  case 0x0b: return op_dec_rr(BC);
  case 0x0c: return op_inc_r(C);
  case 0x0d: return op_dec_r(C);
  case 0x0e: return op_ld_r_n(C);
  case 0x0f: return op_rrca();
  case 0x10: return op_stop();
  case 0x11: return op_ld_rr_nn(DE);
  case 0x12: return op_ld_rr_a(DE);
  case 0x13: return op_inc_rr(DE);
  case 0x14: return op_inc_r(D);
  case 0x15: return op_dec_r(D);
  case 0x16: return op_ld_r_n(D);
  case 0x17: return op_rla();
  case 0x18: return op_jr_n();
  case 0x19: return op_add_hl_rr(DE);
  case 0x1a: return op_ld_a_rr(DE);
  case 0x1b: return op_dec_rr(DE);
  case 0x1c: return op_inc_r(E);
  case 0x1d: return op_dec_r(E);
  case 0x1e: return op_ld_r_n(E);
  case 0x1f: return op_rra();
  case 0x20: return op_jr_f_n(ZF, 0);
  case 0x21: return op_ld_rr_nn(HL);
  case 0x22: return op_ldi_hl_a();
  case 0x23: return op_inc_rr(HL);
  case 0x24: return op_inc_r(H);
  case 0x25: return op_dec_r(H);
  case 0x26: return op_ld_r_n(H);
  case 0x27: return op_daa();
  case 0x28: return op_jr_f_n(ZF, 1);
  case 0x29: return op_add_hl_rr(HL);
  case 0x2a: return op_ldi_a_hl();
  case 0x2b: return op_dec_rr(HL);
  case 0x2c: return op_inc_r(L);
  case 0x2d: return op_dec_r(L);
  case 0x2e: return op_ld_r_n(L);
  case 0x2f: return op_cpl();
  case 0x30: return op_jr_f_n(CF, 0);
  case 0x31: return op_ld_rr_nn(SP);
  case 0x32: return op_ldd_hl_a();
  case 0x33: return op_inc_rr(SP);
  case 0x34: return op_inc_hl();
  case 0x35: return op_dec_hl();
  case 0x36: return op_ld_hl_n();
  case 0x37: return op_scf();
  case 0x38: return op_jr_f_n(CF, 1);
  case 0x39: return op_add_hl_rr(SP);
  case 0x3a: return op_ldd_a_hl();
  case 0x3b: return op_dec_rr(SP);
  case 0x3c: return op_inc_r(A);
  case 0x3d: return op_dec_r(A);
  case 0x3e: return op_ld_r_n(A);
  case 0x3f: return op_ccf();
  case 0x40: return op_ld_r_r(B, B);
  case 0x41: return op_ld_r_r(B, C);
  case 0x42: return op_ld_r_r(B, D);
  case 0x43: return op_ld_r_r(B, E);
  case 0x44: return op_ld_r_r(B, H);
  case 0x45: return op_ld_r_r(B, L);
  case 0x46: return op_ld_r_hl(B);
  case 0x47: return op_ld_r_r(B, A);
  case 0x48: return op_ld_r_r(C, B);
  case 0x49: return op_ld_r_r(C, C);
  case 0x4a: return op_ld_r_r(C, D);
  case 0x4b: return op_ld_r_r(C, E);
  case 0x4c: return op_ld_r_r(C, H);
  case 0x4d: return op_ld_r_r(C, L);
  case 0x4e: return op_ld_r_hl(C);
  case 0x4f: return op_ld_r_r(C, A);
  case 0x50: return op_ld_r_r(D, B);
  case 0x51: return op_ld_r_r(D, C);
  case 0x52: return op_ld_r_r(D, D);
  case 0x53: return op_ld_r_r(D, E);
  case 0x54: return op_ld_r_r(D, H);
  case 0x55: return op_ld_r_r(D, L);
  case 0x56: return op_ld_r_hl(D);
  case 0x57: return op_ld_r_r(D, A);
  case 0x58: return op_ld_r_r(E, B);
  case 0x59: return op_ld_r_r(E, C);
  case 0x5a: return op_ld_r_r(E, D);
  case 0x5b: return op_ld_r_r(E, E);
  case 0x5c: return op_ld_r_r(E, H);
  case 0x5d: return op_ld_r_r(E, L);
  case 0x5e: return op_ld_r_hl(E);
  case 0x5f: return op_ld_r_r(E, A);
  case 0x60: return op_ld_r_r(H, B);
  case 0x61: return op_ld_r_r(H, C);
  case 0x62: return op_ld_r_r(H, D);
  case 0x63: return op_ld_r_r(H, E);
  case 0x64: return op_ld_r_r(H, H);
  case 0x65: return op_ld_r_r(H, L);
  case 0x66: return op_ld_r_hl(H);
  case 0x67: return op_ld_r_r(H, A);
  case 0x68: return op_ld_r_r(L, B);
  case 0x69: return op_ld_r_r(L, C);
  case 0x6a: return op_ld_r_r(L, D);
  case 0x6b: return op_ld_r_r(L, E);
  case 0x6c: return op_ld_r_r(L, H);
  case 0x6d: return op_ld_r_r(L, L);
  case 0x6e: return op_ld_r_hl(L);
  case 0x6f: return op_ld_r_r(L, A);
  case 0x70: return op_ld_hl_r(B);
  case 0x71: return op_ld_hl_r(C);
  case 0x72: return op_ld_hl_r(D);
  case 0x73: return op_ld_hl_r(E);
  case 0x74: return op_ld_hl_r(H);
  case 0x75: return op_ld_hl_r(L);
  case 0x76: return op_halt();
  case 0x77: return op_ld_hl_r(A);
  case 0x78: return op_ld_r_r(A, B);
  case 0x79: return op_ld_r_r(A, C);
  case 0x7a: return op_ld_r_r(A, D);
  case 0x7b: return op_ld_r_r(A, E);
  case 0x7c: return op_ld_r_r(A, H);
  case 0x7d: return op_ld_r_r(A, L);
  case 0x7e: return op_ld_r_hl(A);
  case 0x7f: return op_ld_r_r(A, A);
  case 0x80: return op_add_a_r(B);
  case 0x81: return op_add_a_r(C);
  case 0x82: return op_add_a_r(D);
  case 0x83: return op_add_a_r(E);
  case 0x84: return op_add_a_r(H);
  case 0x85: return op_add_a_r(L);
  case 0x86: return op_add_a_hl();
  case 0x87: return op_add_a_r(A);
  case 0x88: return op_adc_a_r(B);
  case 0x89: return op_adc_a_r(C);
  case 0x8a: return op_adc_a_r(D);
  case 0x8b: return op_adc_a_r(E);
  case 0x8c: return op_adc_a_r(H);
  case 0x8d: return op_adc_a_r(L);
  case 0x8e: return op_adc_a_hl();
  case 0x8f: return op_adc_a_r(A);
  case 0x90: return op_sub_a_r(B);
  case 0x91: return op_sub_a_r(C);
  case 0x92: return op_sub_a_r(D);
  case 0x93: return op_sub_a_r(E);
  case 0x94: return op_sub_a_r(H);
  case 0x95: return op_sub_a_r(L);
  case 0x96: return op_sub_a_hl();
  case 0x97: return op_sub_a_r(A);
  case 0x98: return op_sbc_a_r(B);
  case 0x99: return op_sbc_a_r(C);
  case 0x9a: return op_sbc_a_r(D);
  case 0x9b: return op_sbc_a_r(E);
  case 0x9c: return op_sbc_a_r(H);
  case 0x9d: return op_sbc_a_r(L);
  case 0x9e: return op_sbc_a_hl();
  case 0x9f: return op_sbc_a_r(A);
  case 0xa0: return op_and_a_r(B);
  case 0xa1: return op_and_a_r(C);
  case 0xa2: return op_and_a_r(D);
  case 0xa3: return op_and_a_r(E);
  case 0xa4: return op_and_a_r(H);
  case 0xa5: return op_and_a_r(L);
  case 0xa6: return op_and_a_hl();
  case 0xa7: return op_and_a_r(A);
  case 0xa8: return op_xor_a_r(B);
  case 0xa9: return op_xor_a_r(C);
  case 0xaa: return op_xor_a_r(D);
  case 0xab: return op_xor_a_r(E);
  case 0xac: return op_xor_a_r(H);
  case 0xad: return op_xor_a_r(L);
  case 0xae: return op_xor_a_hl();
  case 0xaf: return op_xor_a_r(A);
  case 0xb0: return op_or_a_r(B);
  case 0xb1: return op_or_a_r(C);
  case 0xb2: return op_or_a_r(D);
  case 0xb3: return op_or_a_r(E);
  case 0xb4: return op_or_a_r(H);
  case 0xb5: return op_or_a_r(L);
  case 0xb6: return op_or_a_hl();
  case 0xb7: return op_or_a_r(A);
  case 0xb8: return op_cp_a_r(B);
  case 0xb9: return op_cp_a_r(C);
  case 0xba: return op_cp_a_r(D);
  case 0xbb: return op_cp_a_r(E);
  case 0xbc: return op_cp_a_r(H);
  case 0xbd: return op_cp_a_r(L);
  case 0xbe: return op_cp_a_hl();
  case 0xbf: return op_cp_a_r(A);
  case 0xc0: return op_ret_f(ZF, 0);
  case 0xc1: return op_pop_rr(BC);
  case 0xc2: return op_jp_f_nn(ZF, 0);
  case 0xc3: return op_jp_nn();
  case 0xc4: return op_call_f_nn(ZF, 0);
  case 0xc5: return op_push_rr(BC);
  case 0xc6: return op_add_a_n();
  case 0xc7: return op_rst_n(0x00);
  case 0xc8: return op_ret_f(ZF, 1);
  case 0xc9: return op_ret();
  case 0xca: return op_jp_f_nn(ZF, 1);
  case 0xcb: return op_cb();
  case 0xcc: return op_call_f_nn(ZF, 1);
  case 0xcd: return op_call_nn();
  case 0xce: return op_adc_a_n();
  case 0xcf: return op_rst_n(0x08);
  case 0xd0: return op_ret_f(CF, 0);
  case 0xd1: return op_pop_rr(DE);
  case 0xd2: return op_jp_f_nn(CF, 0);
  case 0xd3: return op_xx();
  case 0xd4: return op_call_f_nn(CF, 0);
  case 0xd5: return op_push_rr(DE);
  case 0xd6: return op_sub_a_n();
  case 0xd7: return op_rst_n(0x10);
  case 0xd8: return op_ret_f(CF, 1);
  case 0xd9: return op_reti();
  case 0xda: return op_jp_f_nn(CF, 1);
  case 0xdb: return op_xx();
  case 0xdc: return op_call_f_nn(CF, 1);
  case 0xdd: return op_xx();
  case 0xde: return op_sbc_a_n();
  case 0xdf: return op_rst_n(0x18);
  case 0xe0: return op_ld_ffn_a();
  case 0xe1: return op_pop_rr(HL);
  case 0xe2: return op_ld_ffc_a();
  case 0xe3: return op_xx();
  case 0xe4: return op_xx();
  case 0xe5: return op_push_rr(HL);
  case 0xe6: return op_and_a_n();
  case 0xe7: return op_rst_n(0x20);
  case 0xe8: return op_add_sp_n();
  case 0xe9: return op_jp_hl();
  case 0xea: return op_ld_nn_a();
  case 0xeb: return op_xx();
  case 0xec: return op_xx();
  case 0xed: return op_xx();
  case 0xee: return op_xor_a_n();
  case 0xef: return op_rst_n(0x28);
  case 0xf0: return op_ld_a_ffn();
  case 0xf1: return op_pop_rr(AF);
  case 0xf2: return op_ld_a_ffc();
  case 0xf3: return op_di();
  case 0xf4: return op_xx();
  case 0xf5: return op_push_rr(AF);
  case 0xf6: return op_or_a_n();
  case 0xf7: return op_rst_n(0x30);
  case 0xf8: return op_ld_hl_sp_n();
  case 0xf9: return op_ld_sp_hl();
  case 0xfa: return op_ld_a_nn();
  case 0xfb: return op_ei();
  case 0xfc: return op_xx();
  case 0xfd: return op_xx();
  case 0xfe: return op_cp_a_n();
  case 0xff: return op_rst_n(0x38);
  }
}

auto LR35902::instructionCB() -> void {
  switch(auto opcode = read(r[PC]++)) {
  case 0x00: return op_rlc_r(B);
  case 0x01: return op_rlc_r(C);
  case 0x02: return op_rlc_r(D);
  case 0x03: return op_rlc_r(E);
  case 0x04: return op_rlc_r(H);
  case 0x05: return op_rlc_r(L);
  case 0x06: return op_rlc_hl();
  case 0x07: return op_rlc_r(A);
  case 0x08: return op_rrc_r(B);
  case 0x09: return op_rrc_r(C);
  case 0x0a: return op_rrc_r(D);
  case 0x0b: return op_rrc_r(E);
  case 0x0c: return op_rrc_r(H);
  case 0x0d: return op_rrc_r(L);
  case 0x0e: return op_rrc_hl();
  case 0x0f: return op_rrc_r(A);
  case 0x10: return op_rl_r(B);
  case 0x11: return op_rl_r(C);
  case 0x12: return op_rl_r(D);
  case 0x13: return op_rl_r(E);
  case 0x14: return op_rl_r(H);
  case 0x15: return op_rl_r(L);
  case 0x16: return op_rl_hl();
  case 0x17: return op_rl_r(A);
  case 0x18: return op_rr_r(B);
  case 0x19: return op_rr_r(C);
  case 0x1a: return op_rr_r(D);
  case 0x1b: return op_rr_r(E);
  case 0x1c: return op_rr_r(H);
  case 0x1d: return op_rr_r(L);
  case 0x1e: return op_rr_hl();
  case 0x1f: return op_rr_r(A);
  case 0x20: return op_sla_r(B);
  case 0x21: return op_sla_r(C);
  case 0x22: return op_sla_r(D);
  case 0x23: return op_sla_r(E);
  case 0x24: return op_sla_r(H);
  case 0x25: return op_sla_r(L);
  case 0x26: return op_sla_hl();
  case 0x27: return op_sla_r(A);
  case 0x28: return op_sra_r(B);
  case 0x29: return op_sra_r(C);
  case 0x2a: return op_sra_r(D);
  case 0x2b: return op_sra_r(E);
  case 0x2c: return op_sra_r(H);
  case 0x2d: return op_sra_r(L);
  case 0x2e: return op_sra_hl();
  case 0x2f: return op_sra_r(A);
  case 0x30: return op_swap_r(B);
  case 0x31: return op_swap_r(C);
  case 0x32: return op_swap_r(D);
  case 0x33: return op_swap_r(E);
  case 0x34: return op_swap_r(H);
  case 0x35: return op_swap_r(L);
  case 0x36: return op_swap_hl();
  case 0x37: return op_swap_r(A);
  case 0x38: return op_srl_r(B);
  case 0x39: return op_srl_r(C);
  case 0x3a: return op_srl_r(D);
  case 0x3b: return op_srl_r(E);
  case 0x3c: return op_srl_r(H);
  case 0x3d: return op_srl_r(L);
  case 0x3e: return op_srl_hl();
  case 0x3f: return op_srl_r(A);
  case 0x40: return op_bit_n_r(0, B);
  case 0x41: return op_bit_n_r(0, C);
  case 0x42: return op_bit_n_r(0, D);
  case 0x43: return op_bit_n_r(0, E);
  case 0x44: return op_bit_n_r(0, H);
  case 0x45: return op_bit_n_r(0, L);
  case 0x46: return op_bit_n_hl(0);
  case 0x47: return op_bit_n_r(0, A);
  case 0x48: return op_bit_n_r(1, B);
  case 0x49: return op_bit_n_r(1, C);
  case 0x4a: return op_bit_n_r(1, D);
  case 0x4b: return op_bit_n_r(1, E);
  case 0x4c: return op_bit_n_r(1, H);
  case 0x4d: return op_bit_n_r(1, L);
  case 0x4e: return op_bit_n_hl(1);
  case 0x4f: return op_bit_n_r(1, A);
  case 0x50: return op_bit_n_r(2, B);
  case 0x51: return op_bit_n_r(2, C);
  case 0x52: return op_bit_n_r(2, D);
  case 0x53: return op_bit_n_r(2, E);
  case 0x54: return op_bit_n_r(2, H);
  case 0x55: return op_bit_n_r(2, L);
  case 0x56: return op_bit_n_hl(2);
  case 0x57: return op_bit_n_r(2, A);
  case 0x58: return op_bit_n_r(3, B);
  case 0x59: return op_bit_n_r(3, C);
  case 0x5a: return op_bit_n_r(3, D);
  case 0x5b: return op_bit_n_r(3, E);
  case 0x5c: return op_bit_n_r(3, H);
  case 0x5d: return op_bit_n_r(3, L);
  case 0x5e: return op_bit_n_hl(3);
  case 0x5f: return op_bit_n_r(3, A);
  case 0x60: return op_bit_n_r(4, B);
  case 0x61: return op_bit_n_r(4, C);
  case 0x62: return op_bit_n_r(4, D);
  case 0x63: return op_bit_n_r(4, E);
  case 0x64: return op_bit_n_r(4, H);
  case 0x65: return op_bit_n_r(4, L);
  case 0x66: return op_bit_n_hl(4);
  case 0x67: return op_bit_n_r(4, A);
  case 0x68: return op_bit_n_r(5, B);
  case 0x69: return op_bit_n_r(5, C);
  case 0x6a: return op_bit_n_r(5, D);
  case 0x6b: return op_bit_n_r(5, E);
  case 0x6c: return op_bit_n_r(5, H);
  case 0x6d: return op_bit_n_r(5, L);
  case 0x6e: return op_bit_n_hl(5);
  case 0x6f: return op_bit_n_r(5, A);
  case 0x70: return op_bit_n_r(6, B);
  case 0x71: return op_bit_n_r(6, C);
  case 0x72: return op_bit_n_r(6, D);
  case 0x73: return op_bit_n_r(6, E);
  case 0x74: return op_bit_n_r(6, H);
  case 0x75: return op_bit_n_r(6, L);
  case 0x76: return op_bit_n_hl(6);
  case 0x77: return op_bit_n_r(6, A);
  case 0x78: return op_bit_n_r(7, B);
  case 0x79: return op_bit_n_r(7, C);
  case 0x7a: return op_bit_n_r(7, D);
  case 0x7b: return op_bit_n_r(7, E);
  case 0x7c: return op_bit_n_r(7, H);
  case 0x7d: return op_bit_n_r(7, L);
  case 0x7e: return op_bit_n_hl(7);
  case 0x7f: return op_bit_n_r(7, A);
  case 0x80: return op_res_n_r(0, B);
  case 0x81: return op_res_n_r(0, C);
  case 0x82: return op_res_n_r(0, D);
  case 0x83: return op_res_n_r(0, E);
  case 0x84: return op_res_n_r(0, H);
  case 0x85: return op_res_n_r(0, L);
  case 0x86: return op_res_n_hl(0);
  case 0x87: return op_res_n_r(0, A);
  case 0x88: return op_res_n_r(1, B);
  case 0x89: return op_res_n_r(1, C);
  case 0x8a: return op_res_n_r(1, D);
  case 0x8b: return op_res_n_r(1, E);
  case 0x8c: return op_res_n_r(1, H);
  case 0x8d: return op_res_n_r(1, L);
  case 0x8e: return op_res_n_hl(1);
  case 0x8f: return op_res_n_r(1, A);
  case 0x90: return op_res_n_r(2, B);
  case 0x91: return op_res_n_r(2, C);
  case 0x92: return op_res_n_r(2, D);
  case 0x93: return op_res_n_r(2, E);
  case 0x94: return op_res_n_r(2, H);
  case 0x95: return op_res_n_r(2, L);
  case 0x96: return op_res_n_hl(2);
  case 0x97: return op_res_n_r(2, A);
  case 0x98: return op_res_n_r(3, B);
  case 0x99: return op_res_n_r(3, C);
  case 0x9a: return op_res_n_r(3, D);
  case 0x9b: return op_res_n_r(3, E);
  case 0x9c: return op_res_n_r(3, H);
  case 0x9d: return op_res_n_r(3, L);
  case 0x9e: return op_res_n_hl(3);
  case 0x9f: return op_res_n_r(3, A);
  case 0xa0: return op_res_n_r(4, B);
  case 0xa1: return op_res_n_r(4, C);
  case 0xa2: return op_res_n_r(4, D);
  case 0xa3: return op_res_n_r(4, E);
  case 0xa4: return op_res_n_r(4, H);
  case 0xa5: return op_res_n_r(4, L);
  case 0xa6: return op_res_n_hl(4);
  case 0xa7: return op_res_n_r(4, A);
  case 0xa8: return op_res_n_r(5, B);
  case 0xa9: return op_res_n_r(5, C);
  case 0xaa: return op_res_n_r(5, D);
  case 0xab: return op_res_n_r(5, E);
  case 0xac: return op_res_n_r(5, H);
  case 0xad: return op_res_n_r(5, L);
  case 0xae: return op_res_n_hl(5);
  case 0xaf: return op_res_n_r(5, A);
  case 0xb0: return op_res_n_r(6, B);
  case 0xb1: return op_res_n_r(6, C);
  case 0xb2: return op_res_n_r(6, D);
  case 0xb3: return op_res_n_r(6, E);
  case 0xb4: return op_res_n_r(6, H);
  case 0xb5: return op_res_n_r(6, L);
  case 0xb6: return op_res_n_hl(6);
  case 0xb7: return op_res_n_r(6, A);
  case 0xb8: return op_res_n_r(7, B);
  case 0xb9: return op_res_n_r(7, C);
  case 0xba: return op_res_n_r(7, D);
  case 0xbb: return op_res_n_r(7, E);
  case 0xbc: return op_res_n_r(7, H);
  case 0xbd: return op_res_n_r(7, L);
  case 0xbe: return op_res_n_hl(7);
  case 0xbf: return op_res_n_r(7, A);
  case 0xc0: return op_set_n_r(0, B);
  case 0xc1: return op_set_n_r(0, C);
  case 0xc2: return op_set_n_r(0, D);
  case 0xc3: return op_set_n_r(0, E);
  case 0xc4: return op_set_n_r(0, H);
  case 0xc5: return op_set_n_r(0, L);
  case 0xc6: return op_set_n_hl(0);
  case 0xc7: return op_set_n_r(0, A);
  case 0xc8: return op_set_n_r(1, B);
  case 0xc9: return op_set_n_r(1, C);
  case 0xca: return op_set_n_r(1, D);
  case 0xcb: return op_set_n_r(1, E);
  case 0xcc: return op_set_n_r(1, H);
  case 0xcd: return op_set_n_r(1, L);
  case 0xce: return op_set_n_hl(1);
  case 0xcf: return op_set_n_r(1, A);
  case 0xd0: return op_set_n_r(2, B);
  case 0xd1: return op_set_n_r(2, C);
  case 0xd2: return op_set_n_r(2, D);
  case 0xd3: return op_set_n_r(2, E);
  case 0xd4: return op_set_n_r(2, H);
  case 0xd5: return op_set_n_r(2, L);
  case 0xd6: return op_set_n_hl(2);
  case 0xd7: return op_set_n_r(2, A);
  case 0xd8: return op_set_n_r(3, B);
  case 0xd9: return op_set_n_r(3, C);
  case 0xda: return op_set_n_r(3, D);
  case 0xdb: return op_set_n_r(3, E);
  case 0xdc: return op_set_n_r(3, H);
  case 0xdd: return op_set_n_r(3, L);
  case 0xde: return op_set_n_hl(3);
  case 0xdf: return op_set_n_r(3, A);
  case 0xe0: return op_set_n_r(4, B);
  case 0xe1: return op_set_n_r(4, C);
  case 0xe2: return op_set_n_r(4, D);
  case 0xe3: return op_set_n_r(4, E);
  case 0xe4: return op_set_n_r(4, H);
  case 0xe5: return op_set_n_r(4, L);
  case 0xe6: return op_set_n_hl(4);
  case 0xe7: return op_set_n_r(4, A);
  case 0xe8: return op_set_n_r(5, B);
  case 0xe9: return op_set_n_r(5, C);
  case 0xea: return op_set_n_r(5, D);
  case 0xeb: return op_set_n_r(5, E);
  case 0xec: return op_set_n_r(5, H);
  case 0xed: return op_set_n_r(5, L);
  case 0xee: return op_set_n_hl(5);
  case 0xef: return op_set_n_r(5, A);
  case 0xf0: return op_set_n_r(6, B);
  case 0xf1: return op_set_n_r(6, C);
  case 0xf2: return op_set_n_r(6, D);
  case 0xf3: return op_set_n_r(6, E);
  case 0xf4: return op_set_n_r(6, H);
  case 0xf5: return op_set_n_r(6, L);
  case 0xf6: return op_set_n_hl(6);
  case 0xf7: return op_set_n_r(6, A);
  case 0xf8: return op_set_n_r(7, B);
  case 0xf9: return op_set_n_r(7, C);
  case 0xfa: return op_set_n_r(7, D);
  case 0xfb: return op_set_n_r(7, E);
  case 0xfc: return op_set_n_r(7, H);
  case 0xfd: return op_set_n_r(7, L);
  case 0xfe: return op_set_n_hl(7);
  case 0xff: return op_set_n_r(7, A);
  }
}

}
