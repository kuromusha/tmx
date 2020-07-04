/*
	     TMXsave Ver.1.00

	Programmed by K.Kuromusha

	       July 4, 1995
*/

#include	<stdio.h>
#include	<sys/dos.h>

char	text[0x10000],head[0x80],head2[0x80],*fn1,*fn2;
int	size;
short	sum,high=0x500,low=-0x500,x=0,y=0,bufno=0,buf_ptr=0,
	bxx,head_size=0x80,sw=0,swd,
	hyo3[49]={16,17,19,21,23,25,28,31,34,37,41,45,50,
		55,60,66,73,80,88,97,107,118,130,143,157,173,
		190,209,230,253,279,307,337,371,408,449,494,544,598,
		658,724,796,876,963,1060,1166,1282,1411,1552},
	hyo5[49][31],buf[0x1000];
FILE	*fp;
unsigned char	data[2][0x800];

volatile struct DMAREG	*dma;
volatile unsigned char	*ppi_cwr;
volatile unsigned char	*opm_regno;
volatile unsigned char	*opm_data;
volatile unsigned char	*adpcm_command;
volatile unsigned char	*adpcm_data;

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

void	init_hyo()
{
	int	i,a,b,c,d;

	for(i=0; i<49; i++){
		d=hyo3[i];
		a=d>>1;
		b=d>>2;
		c=d>>3;
		hyo5[i][0]=d;
		hyo5[i][1]=a;
		hyo5[i][2]=b;
		hyo5[i][3]=0;
		hyo5[i][4]=c;
		if(i != 0)
			hyo5[i][5]=hyo5[i][8]=hyo5[i][12]=hyo5[i][15]=i-1;
		else
			hyo5[i][5]=hyo5[i][8]=hyo5[i][12]=hyo5[i][15]=0;
		hyo5[i][6]=1;
		hyo5[i][7]=b+c;
		hyo5[i][9]=a+b;
		hyo5[i][10]=2;
		hyo5[i][11]=a+c;
		hyo5[i][13]=3;
		hyo5[i][14]=a+b+c;
		hyo5[i][16]=d+a;
		hyo5[i][17]=d+b;
		hyo5[i][18]=4;
		hyo5[i][19]=d+c;
		if(i < 46)
			hyo5[i][20]=i+2;
		else
			hyo5[i][20]=48;
		hyo5[i][21]=5;
		hyo5[i][22]=d+b+c;
		if(i < 44)
			hyo5[i][23]=i+4;
		else
			hyo5[i][23]=48;
		hyo5[i][24]=d+a+b;
		hyo5[i][25]=6;
		hyo5[i][26]=d+a+c;
		if(i < 42)
			hyo5[i][27]=i+6;
		else
			hyo5[i][27]=48;
		hyo5[i][28]=7;
		hyo5[i][29]=d+a+b+c;
		if(i < 40)
			hyo5[i][30]=i+8;
		else
			hyo5[i][30]=48;
	}

}

void	init()
{
	int	i;

	for(i=0; i<0x800; i++)
		data[1][i]=0x80;

	_dos_super(0);
	dma		= (struct DMAREG *)0xe840c0;
	ppi_cwr		= (unsigned char *)0xe9a007;
	opm_regno	= (unsigned char *)0xe90001;
	opm_data	= (unsigned char *)0xe90003;
	adpcm_command	= (unsigned char *)0xe92001;
	adpcm_data	= (unsigned char *)0xe92003;

	*adpcm_command=1;
	*ppi_cwr=(0<<1) | 0;
	*ppi_cwr=(1<<1) | 0;
	*ppi_cwr=(2<<1) | 0;
	*ppi_cwr=(3<<1) | 1;
	*opm_regno=0x1b;
	*opm_data=0;
	dma->csr=0xff;

	dma->dcr=0x80;
	dma->ocr=0x32;
	dma->scr=0x04;
	dma->ccr=0x00;
	dma->cpr=0x01;
	dma->mfc=0x05;
	dma->dfc=0x05;
	dma->bfc=0x05;
	dma->mtc=0x800;
	dma->btc=0x800;
	dma->mar=data[1];
	dma->bar=data[0];
	dma->dar=(unsigned char *)adpcm_data;
	dma->ccr|=0x80;
	*adpcm_command=2;
	dma->ccr|=0x40;
}

void	sub2(ptr)
unsigned char	*ptr;
{
	short	i,a,b,e,f,*p,*pp;

	pp=buf;
	for(i=0x800; i>0; i--){
		a=*pp++;
		a-=y;
		b=a & 0x7fff;
		p=&hyo5[x][0];
		if(b >= *p++)
			p+=15;
		if(b >= *p++)
			p+=7;
		if(b >= *p++)
			p+=3;
		e=*p++;
		if(a >= 0)
			y+=*p++;
		else {
			e+=8;
			y-=*p++;
		}
		x=*p;
		a=*pp++;
		a-=y;
		b=a & 0x7fff;
		p=&hyo5[x][0];
		if(b >= *p++)
			p+=15;
		if(b >= *p++)
			p+=7;
		if(b >= *p++)
			p+=3;
		f=*p++;
		if(a >= 0)
			y+=*p++;
		else {
			f+=8;
			y-=*p++;
		}
		x=*p;
		*ptr++=(f<<4)|e;
	}
}

void	sub()
{
	sub2(data[bufno]);
	if(dma->csr & 0xd0){
		puts("データ出力用のバッファがあふれました。");
		dma->ccr=0x10;
		*adpcm_command=1;
		exit(-1);
	}
	while(!(dma->csr & 0xd0))
		if(_dos_k_keyinp() == 0x1b){
			puts("強制的に終了します。");
			dma->ccr=0x10;
			*adpcm_command=1;
			exit(-1);
		}
	dma->csr=0xff;
	if(bufno == 0){
		dma->bar=data[1];
		bufno++;
	} else {
		dma->bar=data[0];
		bufno--;
	}
	dma->ccr|=0x40;
}

void	pcm_out(q)
short	q;
{
	int	i;

	buf[buf_ptr++]=q;
	if(buf_ptr >= 0x1000){
		sub();
		buf_ptr=0;
	}
}

void	bitset(z)
int	z;
{
	switch(bxx){
		case	0:
			pcm_out(high);
			pcm_out(high);
			if(z != 0){
				pcm_out(high);
				pcm_out(high);
			}
			pcm_out(low);
			pcm_out(low);
			if(z != 0){
				pcm_out(low);
				pcm_out(low);
			}
			break;
		case	1:
			pcm_out(high);
			pcm_out(high);
			pcm_out(high);
			if(z != 0){
				pcm_out(high);
				pcm_out(high);
			}
			pcm_out(low);
			pcm_out(low);
			if(z != 0){
				pcm_out(low);
				pcm_out(low);
				pcm_out(low);
			}
			break;
		case	2:
			pcm_out(high);
			pcm_out(high);
			pcm_out(high);
			pcm_out(high);
			if(z != 0){
				pcm_out(high);
				pcm_out(high);
				pcm_out(high);
				pcm_out(high);
				pcm_out(high);
			}
			pcm_out(low);
			pcm_out(low);
			pcm_out(low);
			pcm_out(low);
			pcm_out(low);
			if(z != 0){
				pcm_out(low);
				pcm_out(low);
				pcm_out(low);
			}
			break;
		case	3:
			pcm_out(high);
			pcm_out(high);
			if(z != 0){
				pcm_out(high);
				pcm_out(high);
			}
			pcm_out(low);
			pcm_out(low);
			if(z != 0){
				pcm_out(low);
				pcm_out(low);
				pcm_out(low);
			}
	}
}

void	put_header(int x)
{
	int i;

	if(bxx == 0){
		for(i=0; i<5000-2000*x; i++)
			bitset(1);
		for(i=0; i<20*x; i++)
			bitset(0);
		for(i=0; i<20*x; i++)
			bitset(1);
	} else {
		for(i=0; i<5000-2000*x; i++)
			bitset(0);
		for(i=0; i<20*x; i++)
			bitset(1);
		for(i=0; i<20*x; i++)
			bitset(0);
	}
	bitset(1);
}

void	put_byte(int x)
{
	int i;

	bitset(1);
	for(i=0; i<8; i++){
		if((x & 0x80) != 0){
			bitset(1);
			sum++;
		} else
			bitset(0);
		x<<=1;
	}
}

void	put_sum(int x)
{
	put_byte(x>>8 & 0xff);
	put_byte(x & 0xff);
	bitset(1);
}

void	csave()
{
	int i,c;

	init_hyo();
	init();

	put_header(2);
	sum=0;
	for(i=0; i<head_size; i++)
		put_byte(head[i]);
	put_sum(sum);

	put_header(1);
	sum=0;
	for(i=0; i<size; i++)
		put_byte(text[i]);
	put_sum(sum);

	bitset(0);
	while(buf_ptr < 0x1000)
		buf[buf_ptr++]=0;
	sub();
	for(i=0; i<0x1000; i++)
		buf[i]=0;
	sub();
	dma->ccr=0x10;
	*adpcm_command=1;
}

int	sub1(a)
short	a;
{
	int	b,t=0,e;

	a-=y;
	b=a & 0x7fff;
	if(b >= hyo5[x][t++])
		t+=15;
	if(b >= hyo5[x][t++])
		t+=7;
	if(b >= hyo5[x][t++])
		t+=3;
	e=hyo5[x][t++];
	if(a >= 0)
		y+=hyo5[x][t++];
	else {
		e+=8;
		y-=hyo5[x][t++];
	}
	x=hyo5[x][t];
	return(e);
}

void	pcm_out1(q)
short	q;
{
	int	i;

	if( sw == 0 ){
		swd=sub1(q);
		sw++;
	} else {
		swd|=sub1(q)<<4;
		putc(swd,fp);
		sw--;
	}
}

void	bitset1(z)
int	z;
{
	switch(bxx){
		case	0:
			pcm_out1(high);
			pcm_out1(high);
			if(z != 0){
				pcm_out1(high);
				pcm_out1(high);
			}
			pcm_out1(low);
			pcm_out1(low);
			if(z != 0){
				pcm_out1(low);
				pcm_out1(low);
			}
			break;
		case	1:
			pcm_out1(high);
			pcm_out1(high);
			pcm_out1(high);
			if(z != 0){
				pcm_out1(high);
				pcm_out1(high);
			}
			pcm_out1(low);
			pcm_out1(low);
			if(z != 0){
				pcm_out1(low);
				pcm_out1(low);
				pcm_out1(low);
			}
			break;
		case	2:
			pcm_out1(high);
			pcm_out1(high);
			pcm_out1(high);
			pcm_out1(high);
			if(z != 0){
				pcm_out1(high);
				pcm_out1(high);
				pcm_out1(high);
				pcm_out1(high);
				pcm_out1(high);
			}
			pcm_out1(low);
			pcm_out1(low);
			pcm_out1(low);
			pcm_out1(low);
			pcm_out1(low);
			if(z != 0){
				pcm_out1(low);
				pcm_out1(low);
				pcm_out1(low);
			}
			break;
		case	3:
			pcm_out1(high);
			pcm_out1(high);
			if(z != 0){
				pcm_out1(high);
				pcm_out1(high);
			}
			pcm_out1(low);
			pcm_out1(low);
			if(z != 0){
				pcm_out1(low);
				pcm_out1(low);
				pcm_out1(low);
			}
	}
}

void	put_header1(int x)
{
	int i;

	if(bxx == 0){
		for(i=0; i<5000-2000*x; i++)
			bitset1(1);
		for(i=0; i<20*x; i++)
			bitset1(0);
		for(i=0; i<20*x; i++)
			bitset1(1);
	} else {
		for(i=0; i<5000-2000*x; i++)
			bitset1(0);
		for(i=0; i<20*x; i++)
			bitset1(1);
		for(i=0; i<20*x; i++)
			bitset1(0);
	}
	bitset1(1);
}

void	put_byte1(int x)
{
	int i;

	bitset1(1);
	for(i=0; i<8; i++){
		if((x & 0x80) != 0){
			bitset1(1);
			sum++;
		} else
			bitset1(0);
		x<<=1;
	}
}

void	put_sum1(int x)
{
	put_byte1(x>>8 & 0xff);
	put_byte1(x & 0xff);
	bitset1(1);
}

void	csave1()
{
	int i,c;

	init_hyo();

	put_header1(2);
	sum=0;
	for(i=0; i<head_size; i++)
		put_byte1(head[i]);
	put_sum1(sum);

	put_header1(1);
	sum=0;
	for(i=0; i<size; i++)
		put_byte1(text[i]);
	put_sum1(sum);

	bitset1(0);
	fcloseall();
}

void	help()
{
		puts("使い方 : TMXsave [-o<name> -p] <filename>");
		puts("          -o<name> : ADPCMファイル<name>を出力にする");
		puts("          -p       : 位相反転");
		puts("         filename  : 入力ファイル名");
}

void	tape_open()
{
	struct _nameckbuf	buff;
	char			name[128],name2[128],d[]="TMX\15\12\0",
				dn[4][10]={"X1\15\12\0","MZ-2000\15\12\0"
				,"MZ-1200\15\12\0","S-OS\15\12\0"};
	int			i,j;

	if(strlen(fn1) > 123){
		puts("入力ファイル名のパス指定が長すぎます。");
		exit(-1);
	}
	strcpy(name,fn1);
	if(_dos_nameck(fn1,&buff) != 0){
		puts("入力ファイル名が変です。");
		exit(-1);
	}
	if(buff.ext[1] == 0 && fn1[strlen(fn1)-1] != '.')
		strcat(name,".tmx");
	if(fn2 != 0){
		if(strlen(fn2) > 123){
			puts("ADPCMファイル名のパス指定が長すぎます。");
			exit(-1);
		}
		strcpy(name2,fn2);
		if(_dos_nameck(fn2,&buff) != 0){
			puts("ADPCMファイル名が変です。");
			exit(-1);
		}
		if(buff.ext[1] == 0 && fn2[strlen(fn2)-1] != '.')
			strcat(name2,".pcm");
	}
	if((fp=fopen(fn1,"rb")) == NULL)
		if((fp=fopen(name,"rb")) == NULL){
			puts("入力ファイルのオープンに失敗しました。");
			exit(-1);
		}
	if((size=filelength(fileno(fp))) == -1){
		puts("入力ファイルのファイルサイズが得られませんでした。");
		exit(-1);
	}
	size-=256;
	if(size < 0){
		puts("入力ファイルのTMXデータが異常です。");
		exit(-1);
	}
	if(size > 0x10000){
		puts("入力ファイルのTMXデータサイズが６４キロバイトを越えています。");
		exit(-1);
	}
	fread(head2, 1, 0x80, fp);
	fread(head, 1, 0x80, fp);
	fread(text, 1, size, fp);
	fclose(fp);
	for(i=0; d[i]!=0; i++)
		if(head2[i] != d[i]){
			puts("入力ファイルはTMXファイルではありません。");
			exit(-1);
		}
	j=0;
	while(1){
		for(i=0; dn[j][i]!=0; i++)
			if(head2[5+i] != dn[j][i])
				break;
		if(dn[j][i] == 0){
			bxx=j;
			if(bxx == 0)
				head_size=0x20;
			break;
		}
		j++;
		if(j > 3){
			puts("入力ファイルのデータ形式はサポートされていません。");
			exit(-1);
		}
	}
	if((unsigned char)head[18]+((unsigned char)head[19]<<8) != size){
		puts("入力ファイルのヘッダ情報と実際のファイルサイズが異なっています。");
		exit(-1);
	}
	if(fn2 != 0){
		if((fp=fopen(name2,"wb")) == NULL){
			puts("出力ADPCMファイルのオープンに失敗しました。");
			exit(-1);
		}
	} else
		puts("処理を停止する場合は[ESC]を押して下さい。");
}

void	main(int argc, char *argv[])
{
	int	i,j;

	puts("TMXsave Ver.1.00 Copyright 1995 K.Kuromusha");

	fn1=fn2=0;
	for(i=1; i<argc; i++){
		if(argv[i][0] == '-' || argv[i][0] == '/'){
			j=1;
			do {
				switch(tolower(argv[i][j++])){
					case 'o':
						if(argv[i][j] == 0){
							puts("ADPCMファイル名を指定して下さい。");
							exit(-1);
						}
						fn2=&argv[i][j];
						j=-1;
						break;
					case 'p':
						high=-(low=high);
						break;
					default:
						help();
						puts("無効なオプションを指定しました。");
						exit(-1);
				}
			} while(argv[i][j] != 0 && j > 0);
		} else {
			if(fn1 == 0)
				fn1=argv[i];
			else {
				help();
				puts("２つ以上の入力ファイル名を指定しています。");
				exit(-1);
			}
		}
	}

	if(fn1 == 0){
		help();
		exit(0);
	}

	tape_open();
	if(fn2 == 0)
		csave();
	else
		csave1();
}
