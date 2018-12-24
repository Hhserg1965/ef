bb.inc('hh_dom.js')
//bb.inc('a.js')

dbc = bb.db('QPSQL','','hhser','potok','ws_us','websearch')
bb.lg('DB', dbc)
//sq = bb.sql('','select * from ist_inews',true)
sq = bb.sql('','select * from ist_inews')
sq.exec()
var r = false
var cnt = 0
//bb.lg('до НЕКСТА')
while( r = sq.next()) {
//	bb.lg('после НЕКСТА',r)
	bb.lg(r.url,cnt++)
}
bb.lg('sq.rows',sq.rows())

bb.db('QPSQL','cls','hhser','clusterTst','ws_us','websearch')
var s_links3 = bb.sql('cls',"SELECT * FROM links WHERE  id_cls = $1 AND ncl = $2 AND dt > $3 order by dt");

//371,403 227,152
cnt = 0;
s_links3.exec(227,152,'2015-11-20')
while( r = s_links3.next()) {
//	bb.lg('после НЕКСТА',r)
	bb.lg(r.id_txt,r.dt,cnt++)
}
bb.lg('s_links3',s_links3.rows())

i_s = bb.sql('','INSERT INTO t( nm, cnt, t_del, zn) VALUES ($1,$2,$3,$4);')
bl = true
for(i=0; i< 200; ++i){
	i_s.exec('zn_'+i,i,bl,i/0.22)
	bl = !bl
}

sl3.db('rrr.rr',678)


