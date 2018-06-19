

// 推荐快捷键 ctrl+/
//将选择的行添加双斜杠注释或取消双斜杠注释
macro WangQiGuo_MultiLineComment()
{
    hwnd = GetCurrentWnd()
    selection = GetWndSel(hwnd)
    LnFirst = GetWndSelLnFirst(hwnd)      //取首行行号
    LnLast = GetWndSelLnLast(hwnd)      //取末行行号
    hbuf = GetCurrentBuf()
 
    if(GetBufLine(hbuf, 0) == "//magic-number:tph85666031"){
        stop
    }
 
    Ln = Lnfirst
    buf = GetBufLine(hbuf, Ln)
    len = strlen(buf)
 
    while(Ln <= Lnlast) {
        buf = GetBufLine(hbuf, Ln)  //取Ln对应的行
        if(buf == ""){                    //跳过空行
            Ln = Ln + 1
            continue
        }
 
        if(StrMid(buf, 0, 1) == "/") {       //需要取消注释,防止只有单字符的行
            if(StrMid(buf, 1, 2) == "/"){
                PutBufLine(hbuf, Ln, StrMid(buf, 2, Strlen(buf)))
            }
        }
 
        if(StrMid(buf,0,1) != "/"){          //需要添加注释
            PutBufLine(hbuf, Ln, Cat("//", buf))
        }
        Ln = Ln + 1
    }
 
    SetWndSel(hwnd, selection)
}

// 推荐快捷键 ctrl+3
//对选择的代码添加 或取消#if 0  #endif 包围
macro WangQiGuo_AddMacroComment()
{
	hwnd=GetCurrentWnd()
	sel=GetWndSel(hwnd)
	lnFirst=GetWndSelLnFirst(hwnd)
	lnLast=GetWndSelLnLast(hwnd)
	hbuf=GetCurrentBuf()

	if (LnFirst == 0) 
	{
	        szIfStart = ""
	} 
	else 
	{
	        szIfStart = GetBufLine(hbuf, LnFirst-1) //被选择的第一行的上一行的内容
	}
	szIfEnd = GetBufLine(hbuf, lnLast+1) //被选择的代码块的最后一行的下一行内容


	szCodeStart = GetBufLine(hbuf, LnFirst) //被选择的代码块的第一行内容
	szCodeEnd = GetBufLine(hbuf, lnLast)//被选择的代码块的最后一行内容

	start_space_count = 0 //第一行代码的前面的空白个数  只计算Tab个数,忽略空格
	end_space_count = 0  //最后一行的代码的前面的空白个数
	insert_space_count = 0 //我们要插入的#if 0 字符串前面应该插入多少个Tab

	index = 0

	while(index<strlen(szCodeStart))
	{
		if(AsciiFromChar(szCodeStart[index])== 9) //9是Tab字符的ASCII
		{
			start_space_count = start_space_count +1
		}
		index = index + 1 
	}

	index = 0
	while(index<strlen(szCodeEnd))
	{
		if(AsciiFromChar(szCodeEnd[index])== 9)
		{
			end_space_count=end_space_count+1
		}
		index = index + 1 
	}

	//代码块的第一行和最后一行前面的Tab个数,取比较小的那个值
	if(start_space_count<end_space_count)
	{
		insert_space_count = start_space_count -1
	}
	else
	{
		insert_space_count = end_space_count -1
	}

	str_start_insert=""
	str_end_insert=""
	index=0

	while(index<insert_space_count)
	{
		str_start_insert=str_start_insert#"	"  //这里添加的Tab字符
		str_end_insert=str_end_insert#"	"  //这里添加的也是Tab字符
		index = index + 1 
	}
	str_start_insert=str_start_insert#"#if 0"    //在#if 0 开始符号和结束符号前都添加Tab字符,比代码行前面的空白少一个
	str_end_insert=str_end_insert#"#endif"    

	if (_WangQiGuo_TrimString(szIfStart) == "#if 0" && _WangQiGuo_TrimString(szIfEnd) =="#endif") {
	        DelBufLine(hbuf, lnLast+1)
	        DelBufLine(hbuf, lnFirst-1)
	        sel.lnFirst = sel.lnFirst - 1
	        sel.lnLast = sel.lnLast - 1
	} 
	else 
	{
	        InsBufLine(hbuf, lnFirst, str_start_insert)
	        InsBufLine(hbuf, lnLast+2, str_end_insert)
	        sel.lnFirst = sel.lnFirst + 1
	        sel.lnLast = sel.lnLast + 1
	}
	SetWndSel( hwnd, sel )
}

// 推荐快捷键 ctrl+8
//将选中的代码块添加多行注释  
macro WangQiGuo_CommentSelStr()
{
	hwnd=GetCurrentWnd()
	sel=GetWndSel(hwnd)
	lnFirst=GetWndSelLnFirst(hwnd)
	lnLast=GetWndSelLnLast(hwnd)
	hbuf=GetCurrentBuf()

	if (LnFirst == 0) 
	{
	        szIfStart = ""
	} 
	else 
	{
	        szIfStart = GetBufLine(hbuf, LnFirst-1) //被选择的第一行的上一行的内容
	}
	szIfEnd = GetBufLine(hbuf, lnLast+1) //被选择的代码块的最后一行的下一行内容


	szCodeStart = GetBufLine(hbuf, LnFirst) //被选择的代码块的第一行内容
	szCodeEnd = GetBufLine(hbuf, lnLast)//被选择的代码块的最后一行内容

	start_space_count = 0 //第一行代码的前面的空白个数  只计算Tab个数,忽略空格
	end_space_count = 0  //最后一行的代码的前面的空白个数
	insert_space_count = 0

	index = 0

	while(index<strlen(szCodeStart))
	{
		if(AsciiFromChar(szCodeStart[index])== 9) //9是Tab字符的ASCII
		{
			start_space_count = start_space_count +1
		}
		index = index + 1 
	}

	index = 0
	while(index<strlen(szCodeEnd))
	{
		if(AsciiFromChar(szCodeEnd[index])== 9)
		{
			end_space_count=end_space_count+1
		}
		index = index + 1 
	}

	if(start_space_count<end_space_count)
	{
		insert_space_count = start_space_count -1
	}
	else
	{
		insert_space_count = end_space_count -1
	}

	str_start_insert=""
	str_end_insert=""
	index=0

	while(index<insert_space_count)
	{
		str_start_insert=str_start_insert#"	"  //这里添加的Tab字符
		str_end_insert=str_end_insert#"	"
		index = index + 1 
	}
	str_start_insert=str_start_insert#"/*"    //在注释开始符号和结束符号前都添加Tab字符,比代码行前面的空白少一个
	str_end_insert=str_end_insert#"*/"    

	if (_WangQiGuo_TrimString(szIfStart) == "/*" && _WangQiGuo_TrimString(szIfEnd) =="*/") {
	        DelBufLine(hbuf, lnLast+1)
	        DelBufLine(hbuf, lnFirst-1)
	        sel.lnFirst = sel.lnFirst - 1
	        sel.lnLast = sel.lnLast - 1
	} else {
	        InsBufLine(hbuf, lnFirst, str_start_insert)
	        InsBufLine(hbuf, lnLast+2, str_end_insert)
	        sel.lnFirst = sel.lnFirst + 1
	        sel.lnLast = sel.lnLast + 1
	}

	SetWndSel( hwnd, sel )
}



//去掉左边空格,Tab等
macro _WangQiGuo_TrimLeft(szLine)
{
    nLen = strlen(szLine)
    if(nLen == 0)
    {
        return szLine
    }
    nIdx = 0
    while( nIdx < nLen )
    {
        if( ( szLine[nIdx] != " ") && (szLine[nIdx] != "\t") )
        {
            break
        }
        nIdx = nIdx + 1
    }
    return strmid(szLine,nIdx,nLen)
}


//去掉字符串右边的空格
macro _WangQiGuo_TrimRight(szLine)
{
    nLen = strlen(szLine)
    if(nLen == 0)
    {
        return szLine
    }
    nIdx = nLen
    while( nIdx > 0 )
    {
        nIdx = nIdx - 1
        if( ( szLine[nIdx] != " ") && (szLine[nIdx] != "\t") )
        {
            break
        }
    }
    return strmid(szLine,0,nIdx+1)
}

//去掉字符串两边空格
macro _WangQiGuo_TrimString(szLine)
{
    szLine = TrimLeft(szLine)
    szLIne = TrimRight(szLine)
    return szLine
}


