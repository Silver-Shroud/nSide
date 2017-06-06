auto WDC65816::serialize(serializer& s) -> void {
  s.integer(r.pc.d);

  s.integer(r.a.w);
  s.integer(r.x.w);
  s.integer(r.y.w);
  s.integer(r.z.w);
  s.integer(r.s.w);
  s.integer(r.d.w);

  s.integer(r.p.b);

  s.integer(r.db);
  s.integer(r.e);
  s.integer(r.irq);
  s.integer(r.wai);
  s.integer(r.stp);
  s.integer(r.mdr);
  s.integer(r.vector);

  s.integer(aa.d);
  s.integer(rd.d);
  s.integer(sp);
  s.integer(dp);
}
