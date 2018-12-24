
if( ARGV[0] == 'ej')
	levdb = bb.ld('liblevw.so')
else
	levdb = bb.ld('levw')

//bb.lg(ARGV[1])
t_db = levdb.open(ARGV[1])
//bb.lg('t_db',t_db)

//t_db.set('test','value')

f = t_db.find()
//bb.lg('f',f)
cnt = 0
var r = false
while( f.valid()){
//bb.lg('r',r)
	if( bb.mq()) break
//bb.lg('r',r)

	var k = f.k()

	var kd = f.kd()

	var v = f.v()

/*
	if( cnt < 100000) {
		bb.lg(k,v,cnt,'\n');
	}else{
		if( (cnt % 100000) == 0) bb.lg(k,v,cnt,'\n');
	}
*/
	bb.lg(cnt,k,'<',kd,'>','->',v,'\n');

	++cnt

	//--
	f.next()
//	f.prev()
}
f.close()

t_db.close()
bb.lg('cnt:',cnt)