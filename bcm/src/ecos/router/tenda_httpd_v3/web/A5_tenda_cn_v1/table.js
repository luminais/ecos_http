function tr_onmouseover(){
 eval("this.className='a3';");
}
function tr_onmouseout_grey(){
 eval("this.className='a1';");
}
function tr_onmouseout_white(){
 eval("this.className='a2';");
}
function table_onload(tb){
 var i;
 var tlist=document.getElementById(tb);
 for(i=0;i<tlist.rows.length;i++){
  if(i%2==0){
	tlist.rows[i].className="a1";
	tlist.rows[i].onmouseout=tr_onmouseout_grey;
  }
  else{
	tlist.rows[i].className="a2";
	tlist.rows[i].onmouseout=tr_onmouseout_white;
  }
  tlist.rows[i].onmouseover=tr_onmouseover;
 }
}
function table_onload1(tb){
 var i;
 var tlist=document.getElementById(tb);
 for(i=0;i<tlist.rows.length;i++){
  if(i%2==0){
	tlist.rows[i].className="a2";
	tlist.rows[i].onmouseout=tr_onmouseout_white;
  }
  else{
	tlist.rows[i].className="a1";
	tlist.rows[i].onmouseout=tr_onmouseout_grey;
  }
  tlist.rows[i].onmouseover=tr_onmouseover;
 }
}
function table_onload2(tb){
 var i;
 var tlist=document.getElementById(tb);
 for(i=0;i<tlist.rows.length;i++){
  if(i%2==0){
	tlist.rows[i].className="a2";
  }
  else{
	tlist.rows[i].className="a1";
  }
 }
}
