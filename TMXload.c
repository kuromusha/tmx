/*
	     TMXload Ver.1.00

	Programmed by K.Kuromusha

	       July 2, 1995
*/

#include	<stdio.h>
#include	<sys/dos.h>

char		*fn2,*fn3,text[0x10000],head[0x80],head2[0x80],name[128];
FILE		*fp2,*fp3;
int		ptr=0;
short		data,sum,ptr_i=0,p=-1,flag=0,x=0,y=0,d_ptr=0,s=0,f=0,head_p=0,z=0,
		bufno=0,bx=0,bxx,head_size=0x80,
		hyo5[256][49][2],hyo6[256][49],data2[0x1000];
unsigned char	data1[2][0x800];

volatile struct DMAREG	*dma;
volatile unsigned char	*ppi_cwr;
volatile unsigned char	*opm_regno;
volatile unsigned char	*opm_data;
volatile unsigned char	*adpcm_command;
volatile unsigned char	*adpcm_data;

void	error();

struct	DMAREG	{
		unsigned char	csr;
		unsigned char	cer;
		unsigned short	spare1;
		unsigned char	dcr;
		unsigned char	ocr;
		unsigned char	scr;
		unsigned char	ccr;
		unsigned short	spare2;
		unsigned short	mtc;
		unsigned char	*mar;
		unsigned long	spare3;
		unsigned char	*dar;
		unsigned short	spare4;
		unsigned short	btc;
		unsigned char	*bar;
		unsigned long	spare5;
		unsigned char	spare6;
		unsigned char	niv;
		unsigned char	spare7;
		unsigned char	eiv;
		unsigned char	spare8;
		unsigned char	mfc;
		unsigned short	spare9;
		unsigned char	spare10;
		unsigned char	cpr;
		unsigned short	spare11;
		unsigned char	spare12;
		unsigned char	dfc;
		unsigned long	spare13;
		unsigned short	spare14;
		unsigned char	spare15;
		unsigned char	bfc;
		unsigned long	spare16;
		unsigned char	spare17;
		unsigned char	gcr;
	};

void	adp2pcm()
{
	int	a,b,i;

	for(i=0; i<0x800; i++){
		a=data1[bufno][i];
		y+=hyo5[a][x][0];
		data2[i<<1]=y^p;
		y+=hyo5[a][x][1];
		data2[(i<<1)+1]=y^p;
		x=hyo6[a][x];
	}
}

void	init_hyo()
{
	int	b,c,d,i,j,k;
	short	hyo3[49]={16,17,19,21,23,25,28,31,34,37,41,45,50,
			55,60,66,73,80,88,97,107,118,130,143,157,173,
			190,209,230,253,279,307,337,371,408,449,494,544,598,
			658,724,796,876,963,1060,1166,1282,1411,1552},
		hyo4[16]={-1,-1,-1,-1,2,4,6,8,-1,-1,-1,-1,2,4,6,8};

	for(i=0; i<16; i++)
		for(d=0; d<16; d++)
			for(j=0; j<49; j++){
				c=0;
				b=hyo3[j];
				if( i & 4 )
					c+=b;
				b>>=1;
				if( i & 2 )
					c+=b;
				b>>=1;
				if( i & 1 )
					c+=b;
				b>>=1;
				c+=b;
				if( i & 8 )
					c*=-1;
				hyo5[i | (d << 4)][j][0]=c;
				k=j+hyo4[i];
				if( k < 0 )
					k=0;
				else
					if( k > 48 )
						k=48;
				c=0;
				b=hyo3[k];
				if( d & 4 )
					c+=b;
				b>>=1;
				if( d & 2 )
					c+=b;
				b>>=1;
				if( d & 1 )
					c+=b;
				b>>=1;
				c+=b;
				if( d & 8 )
					c*=-1;
				hyo5[i | (d << 4)][j][1]=c;
				k=k+hyo4[d];
				if( k < 0 )
					k=0;
				else
					if( k > 48 )
						k=48;
				hyo6[i | (d << 4)][j]=k;
			}

}

void	init()
{
	_dos_super(0);
	dma		= (struct DMAREG *)0xe840c0;
	ppi_cwr		= (unsigned char *)0xe9a007;
	opm_regno	= (unsigned char *)0xe90001;
	opm_data	= (unsigned char *)0xe90003;
	adpcm_command	= (unsigned char *)0xe92001;
	adpcm_data	= (unsigned char *)0xe92003;

	*adpcm_command=1;
	*ppi_cwr=(0<<1) | s;
	*ppi_cwr=(1<<1) | s;
	*ppi_cwr=(2<<1) | 0;
	*ppi_cwr=(3<<1) | 1;
	*opm_regno=0x1b;
	*opm_data=0;
	dma->csr=0xff;

	dma->dcr=0x80;
	dma->ocr=0xb2;
	dma->scr=0x04;
	dma->ccr=0x00;
	dma->cpr=0x01;
	dma->mfc=0x05;
	dma->dfc=0x05;
	dma->bfc=0x05;
	dma->mtc=0x800;
	dma->btc=0x800;
	dma->mar=data1[0];
	dma->bar=data1[1];
	dma->dar=(unsigned char *)adpcm_data;
	dma->ccr|=0x80;
	*adpcm_command=4;
	dma->ccr|=0x40;
	while(!(dma->csr & 0xd0));
	dma->csr=0xff;
	dma->bar=data1[0];
	dma->ccr|=0x40;
	adp2pcm();
}

void	get_from_pcm()
{
	if(dma->csr & 0xd0){
		puts("データ読み込み用のバッファがあふれました。");
		dma->ccr=0x10;
		*adpcm_command=1;
		error(2);
	}
	while(!(dma->csr & 0xd0))
		if(_dos_k_keyinp() == 0x1b){
			puts("強制的に終了します。");
			error(2);
		}
	dma->csr=0xff;
	if(bufno == 0){
		dma->bar=data1[1];
		bufno++;
	} else {
		dma->bar=data1[0];
		bufno--;
	}
	dma->ccr|=0x40;
	adp2pcm();
}

void	error(int x)
{
	if(fn3 == 0){
		dma->ccr=0x10;
		*adpcm_command=1;
	}
	if(x == 0)
		puts("サンプリングデータが途中で終わっています。");
	else
		if(x == 1)
			puts("チェックサムエラーです。");
	if(head_p != 0){
		fwrite(head2, 1, 0x80, fp2);
		fwrite(head, 1, 0x80, fp2);
		fwrite(text, 1, ptr, fp2);
		puts("不完全な出力ファイルを生成しました。");
	} else {
		fclose(fp2);
		unlink(name);
	}
	exit(-1);
}

int	fget_word()
{
	if( flag == 0 ){
		if(d_ptr >= 0x1000){
			get_from_pcm();
			d_ptr=0;
		}
		data=data2[d_ptr++];
	} else
		flag--;
	return(data);
}

int	get_bit()
{
	int	c=0;

	while(fget_word() < 0);
	while(fget_word() > 0)
		c++;
	while(fget_word() < 0)
		c++;
	flag++;
	if( c >= bxx )
		return(1);
	else
		return(0);
}

void	get_header(int x)
{
	int	i=0;

	while(abs(fget_word())<100);
	flag++;
	if(bx == 0)
		while(i == 0){
			for(; i<20*x; i++)
				if(get_bit() != 0)
					i=-1;
			for(i=0; i<20*x; i++)
				if(get_bit() == 0){
					i=0;
					break;
				}
		}
	else
		while(i == 0){
			for(; i<20*x; i++)
				if(get_bit() == 0)
					i=-1;
			for(i=0; i<20*x; i++)
				if(get_bit() != 0){
					i=0;
					break;
				}
		}
	get_bit();
	get_bit();
}

int	get_byte()
{
	int	i,x=0;

	for(i=0; i<8; i++){
		x<<=1;
		if(get_bit() != 0){
			x++;
			sum++;
		}
	}
	get_bit();
	return(x);
}

int	check_sum(short x)
{
	if((get_byte()<<8)+get_byte() != x)
		return(1);
	return(0);
}

int	bytes2word(unsigned char x,unsigned char y)
{
	return(x+(y<<8));
}

void	cload()
{
	int	i,size,start;

	init_hyo();
	init();
	get_header(2);
	sum=0;
	while(ptr_i < head_size)
		head[ptr_i++]=get_byte();
	if(check_sum(sum))
		error(1);

	for(i=1; i<17; i++) if(head[i] == 13) break;
	head[i]=0;
	size =bytes2word(head[18],head[19]);
	start=bytes2word(head[20],head[21]);
	printf(" ファイルの属性  : ");
	switch(head[0]){
		case 1:
			puts("BIN");
			break;
		case 2:
			puts("BAS");
			break;
		case 4:
			puts("ASC");
			break;
		default:
			puts("???");
	}
	printf(" ファイルネーム  : %s\n",&head[1]);
	printf("スタートアドレス : %.4X\n",start);
	printf(" エンド アドレス : %.4X\n",(start+size & 0xffff)-1);
	printf("ジャンプアドレス : %.4X\n",bytes2word(head[22],head[23]));
	head_p=1;

	get_header(1);
	puts("データを読み込んでいます。");
	sum=0;
	while(ptr < size)
		text[ptr++]=get_byte();
	if(check_sum(sum) != 0)
		error(1);
	dma->ccr=0x10;
	*adpcm_command=1;
}

int	fget_word1()
{
	int	a;

	if( flag == 0 ){
		if(z == 0){
			if( (a=fgetc(fp3)) == EOF )
				error(0);
			y+=hyo5[a][x][0];
			data=y^p;
			y+=hyo5[a][x][1];
			x=hyo6[a][x];
			z++;
		} else {
			data=y^p;
			z--;
		}
	} else
		flag--;
	return(data);
}

int	get_bit1()
{
	int	c=0;

	while(fget_word1() < 0);
	while(fget_word1() > 0)
		c++;
	while(fget_word1() < 0)
		c++;
	flag++;
	if( c >= bxx )
		return(1);
	else
		return(0);
}

void	get_header1(int x)
{
	int	i=0;

	while(abs(fget_word1())<100);
	flag++;
	if(bx == 0)
		while(i == 0){
			for(; i<20*x; i++)
				if(get_bit1() != 0)
					i=-1;
			for(i=0; i<20*x; i++)
				if(get_bit1() == 0){
					i=0;
					break;
				}
		}
	else
		while(i == 0){
			for(; i<20*x; i++)
				if(get_bit1() == 0)
					i=-1;
			for(i=0; i<20*x; i++)
				if(get_bit1() != 0){
					i=0;
					break;
				}
		}
	get_bit1();
	get_bit1();
}

int	get_byte1()
{
	int	i,x=0;

	for(i=0; i<8; i++){
		x<<=1;
		if(get_bit1() != 0){
			x++;
			sum++;
		}
	}
	get_bit1();
	return(x);
}

int	check_sum1(short x)
{
	if((get_byte1()<<8)+get_byte1() != x)
		return(1);
	return(0);
}

void	cload1()
{
	int	i,size,start;

	init_hyo();
	get_header1(2);
	sum=0;
	while(ptr_i < head_size)
		head[ptr_i++]=get_byte1();
	if(check_sum1(sum))
		error(1);

	for(i=1; i<17; i++) if(head[i] == 13)
		break;
	head[i]=0;
	size =bytes2word(head[18],head[19]);
	start=bytes2word(head[20],head[21]);
	printf(" ファイルの属性  : ");
	switch(head[0]){
		case 1:
			puts("BIN");
			break;
		case 2:
			puts("BAS");
			break;
		case 4:
			puts("ASC");
			break;
		default:
			puts("???");
	}
	printf(" ファイルネーム  : %s\n",&head[1]);
	printf("スタートアドレス : %.4X\n",start);
	printf(" エンド アドレス : %.4X\n",(start+size & 0xffff)-1);
	printf("ジャンプアドレス : %.4X\n",bytes2word(head[22],head[23]));
	head_p=1;

	get_header1(1);
	puts("データを読み込んでいます。");
	sum=0;
	while(ptr < size)
		text[ptr++]=get_byte1();
	if(check_sum1(sum) != 0)
		error(1);
}

void	help()
{
		puts("使い方 : TMXload [-b? -i<name> -s -p] <filename>");
		puts("          -b0      : Ｘ１フォーマット（デフォルト）");
		puts("          -b1      : ＭＺフォーマット（２０００ボー）");
		puts("          -b2      : ＭＺフォーマット（１２００ボー）");
		puts("          -b3      : Ｓ－ＯＳフォーマット");
		puts("          -i<name> : ADPCMファイル<name>を入力にする");
		puts("          -s       : スピーカーに音を出力しない");
		puts("          -p       : 位相反転");
		puts("         filename  : 出力ファイル名");
}

void	init0()
{
	int	i=0;

	head2[i++]='T';
	head2[i++]='M';
	head2[i++]='X';
	head2[i++]=0x0d;
	head2[i++]=0x0a;
	switch(bx){
		case	0:
			bxx=4;
			head_size=0x20;
			head2[i++]='X';
			head2[i++]='1';
			break;
		case	1:
			bxx=6;
			head2[i++]='M';
			head2[i++]='Z';
			head2[i++]='-';
			head2[i++]='2';
			head2[i++]='0';
			head2[i++]='0';
			head2[i++]='0';
			break;
		case	2:
			bxx=11;
			head2[i++]='M';
			head2[i++]='Z';
			head2[i++]='-';
			head2[i++]='1';
			head2[i++]='2';
			head2[i++]='0';
			head2[i++]='0';
			break;
		case	3:
			bxx=5;
			head2[i++]='S';
			head2[i++]='-';
			head2[i++]='O';
			head2[i++]='S';
	}
	head2[i++]=0x0d;
	head2[i++]=0x0a;
	head2[i++]=0x1a;
	while(i < 0x80)
		head2[i++]=0;
	if(bx == 0)
		for(i=0x20; i<0x80; i++)
			head[i]=0;
}

void	tape_open()
{
	struct _nameckbuf	buff;
	char			name2[128];

	if(strlen(fn2) > 123){
		puts("出力ファイル名のパス指定が長すぎます。");
		exit(-1);
	}
	strcpy(name,fn2);
	if(_dos_nameck(fn2,&buff) != 0){
		puts("出力ファイル名が変です。");
		exit(-1);
	}
	if(buff.ext[1] == 0 && fn2[strlen(fn2)-1] != '.')
		strcat(name,".tmx");
	if(fn3 != 0){
		if(strlen(fn3) > 123){
			puts("ADPCMファイル名のパス指定が長すぎます。");
			exit(-1);
		}
		strcpy(name2,fn3);
		if(_dos_nameck(fn3,&buff) != 0){
			puts("ADPCMファイル名が変です。");
			exit(-1);
		}
		if(buff.ext[1] == 0 && fn3[strlen(fn3)-1] != '.')
			strcat(name2,".pcm");
	}
	if((fp2=fopen(name,"wb")) == NULL){
		puts("出力ファイルのオープンに失敗しました。");
		exit(-1);
	}
	if(fn3 != 0){
		if((fp3=fopen(fn3,"rb")) == NULL)
			if((fp3=fopen(name2,"rb")) == NULL){
				puts("ADPCMファイルのオープンに失敗しました。");
				fclose(fp2);
				unlink(name);
				exit(-1);
			}
	} else
		puts("処理を停止する場合は[ESC]を押して下さい。");
}

void	tape_close()
{
	fwrite(head2, 1, 0x80, fp2);
	fwrite(head, 1, 0x80, fp2);
	fwrite(text, 1, ptr, fp2);
	fcloseall();
}

void	main(int argc, char *argv[])
{
	int	i,j;

	puts("TMXload Ver.1.00 Copyright 1995 K.Kuromusha");

	fn2=fn3=0;
	for(i=1; i<argc; i++){
		if(argv[i][0] == '-' || argv[i][0] == '/'){
			j=1;
			do {
				switch(tolower(argv[i][j++])){
					case 'b':
						bx=argv[i][j++]-'0';
						if(bx < 0 || bx > 3){
							puts("無効なフォーマットを指定しています。");
							exit(-1);
						}
						break;
					case 'i':
						if(argv[i][j] == 0){
							puts("ADPCMファイル名を指定して下さい。");
							exit(-1);
						}
						fn3=&argv[i][j];
						j=-1;
						break;
					case 's':
						s=1;
						break;
					case 'p':
						p=0;
						break;
					default:
						help();
						puts("無効なオプションを指定しました。");
						exit(-1);
				}
			} while(argv[i][j] != 0 && j > 0);
		} else {
			if(fn2 == 0)
				fn2=argv[i];
			else {
				help();
				puts("２つ以上の出力ファイル名を指定しています。");
				exit(-1);
			}
		}
	}

	if(fn2 == 0){
		help();
		exit(0);
	}

	init0();
	tape_open();
	if(fn3 == 0)
		cload();
	else
		cload1();
	tape_close();
}
