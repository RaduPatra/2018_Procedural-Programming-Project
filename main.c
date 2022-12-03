//Proiect PP - Patrascoiu Radu - Grupa 133 - Dec 2018
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#define min(a,b) (((a)<(b))?(a):(b))
#define max(a,b) (((a)>(b))?(a):(b))
typedef struct
{
    unsigned int W,H;
    unsigned char B,G,R;

} pixel;
typedef struct///struct pt calc chi_squared
{
    unsigned int fB,fG,fR;
} frecvpx;
typedef struct
{
    int idxL,idxC;
    float corr;
    char *nume;
} corel;
void elim_endl(char **nume_img)
{
    int line_len=strlen(*nume_img);
    if ((*nume_img)[line_len - 1] == '\n')
    {
        (*nume_img)[line_len - 1] = '\0';
        line_len -= 1;
    }
}
void exit_if_null(char *nume_img, FILE *f)
{
    if (f == NULL)
    {
        fprintf(stderr, "Nu am putut deschide fisierul %s.\n", nume_img);
        exit(-1);
    }
}
void citire(char **nume_img, char **nume_img_enc,char **nume_img_dec,char **secret_key,char **nume_TM,char **templates,char **img_matches)
{
    *nume_img=(char*)malloc(50);
    *nume_img_enc=(char*)malloc(50);
    *nume_img_dec=(char*)malloc(50);
    *secret_key=(char*)malloc(50);
    *nume_TM=(char*)malloc(50);
    *templates=(char*)malloc(50);
    *img_matches=(char*)malloc(50);

    printf("Citeste calea imaginilor! \n\n");
    printf("Calea imaginii de criptat: ");
    fgets(*nume_img,50,stdin);
    printf("Calea imaginii in care va fi salvata imaginea criptata: ");
    fgets(*nume_img_enc,50,stdin);
    printf("Calea imaginii in care va fi salvata imaginea decriptata: ");
    fgets(*nume_img_dec,50,stdin);
    printf("Calea fisierului in care se afla cheia: ");
    fgets(*secret_key,50,stdin);

    printf("Calea imaginii pt template matching: ");
    fgets(*nume_TM,50,stdin);
    printf("Calea imaginii in care se vor desena detectiile: ");
    fgets(*img_matches,50,stdin);
    printf("Calea fisierului cu sabloane: ");
    fgets(*templates,50,stdin);

    elim_endl(&*nume_img);
    elim_endl(&*nume_img_enc);
    elim_endl(&*nume_img_dec);
    elim_endl(&*secret_key);
    elim_endl(&*nume_TM);
    elim_endl(&*templates);
    elim_endl(&*img_matches);
}
void grayscale_image(char* nume_fisier_sursa,char* nume_fisier_destinatie)
{
    FILE *fin, *fout;
    unsigned int dim_img, latime_img, inaltime_img;
    unsigned char pRGB[3], aux;

    fin = fopen(nume_fisier_sursa, "rb");
    exit_if_null(nume_fisier_sursa, fin);
    fout = fopen(nume_fisier_destinatie, "wb+");

    fseek(fin, 2, SEEK_SET);
    fread(&dim_img, sizeof(unsigned int), 1, fin);

    fseek(fin, 18, SEEK_SET);
    fread(&latime_img, sizeof(unsigned int), 1, fin);
    fread(&inaltime_img, sizeof(unsigned int), 1, fin);

    //copiaza octet cu octet imaginea initiala in cea noua
    fseek(fin,0,SEEK_SET);
    unsigned char c;
    while(fread(&c,1,1,fin)==1)
    {
        fwrite(&c,1,1,fout);
        fflush(fout);
    }

    fclose(fin);

    //calculam padding-ul pentru o linie
    int padding;
    if(latime_img % 4 != 0)
        padding = 4 - (3 * latime_img) % 4;
    else
        padding = 0;

    fseek(fout, 54, SEEK_SET);
    int i,j;
    for(i = 0; i < inaltime_img; i++)
    {
        for(j = 0; j < latime_img; j++)
        {
            //citesc culorile pixelului
            fread(pRGB, 3, 1, fout);
            //fac conversia in pixel gri
            aux = 0.299*pRGB[2] + 0.587*pRGB[1] + 0.114*pRGB[0];
            pRGB[0] = pRGB[1] = pRGB[2] = aux;
            fseek(fout, -3, SEEK_CUR);
            fwrite(pRGB, 3, 1, fout);
            fflush(fout);
        }
        fseek(fout,padding,SEEK_CUR);
    }

    fclose(fout);
}
unsigned int *XORSHIFT32 (unsigned int n,unsigned int seed)///n=2*W*H-1
{
    unsigned r=seed,k,*random;
    random=(unsigned int*)malloc((1+n)*sizeof(unsigned int));
    random[0]=seed;
    for (k=0; k<n; k++)
    {
        r ^= r << 13;
        r ^= r >> 17;
        r ^= r << 5;
        random[k+1]=r;
    }
    return random;
}

unsigned int *Durst (unsigned int n,unsigned int *random)
{
    unsigned int i,k=1,r,aux;
    unsigned int *perm;
    perm=(unsigned int*)malloc(n*sizeof(unsigned int));
    for (i=0; i<n; i++)
        perm[i]=i;

    for (i=n-1; i>=1; i--)
    {
        r=random[k]%(i+1);
        aux=perm[r];
        perm[r]=perm[i];
        perm[i]=aux;
        k++;
    }
    return perm;
}
void copyHeader(char *nume_img,char *nume_dest)
{
    FILE *fin,*fout;
    int i;
    unsigned char c;
    fin= fopen(nume_img,"rb");
    exit_if_null(nume_img,fin);
    fout = fopen(nume_dest, "wb");

    for (i=0; i<54; i++)
    {
        fread(&c,1,1,fin);
        fwrite(&c,1,1,fout);
        fflush(fout);
    }
    fclose(fin);
    fclose(fout);
}

void getSize(FILE *f, unsigned int *latime,unsigned int *inaltime)
{
    fseek(f, 18, SEEK_SET);
    fread(&*latime, sizeof(unsigned int), 1, f);
    fread(&*inaltime, sizeof(unsigned int), 1, f);
}

unsigned int *perm_inv(unsigned int *perm,unsigned int n)
{
    unsigned int i;
    unsigned int *inv;
    inv=(unsigned int*)malloc(n*sizeof(unsigned int));

    for (i = 0; i < n; i++)
        inv[perm[i]] = i;

    return inv;
}

pixel* liniarize(char *nume_img)
{
    FILE *fin;
    unsigned int latime, inaltime;
    int i,j;
    unsigned char pRGB[3];

    fin= fopen(nume_img,"rb");
    exit_if_null(nume_img,fin);

    ///header - dimensiuni
    getSize(fin,&latime,&inaltime);
    fseek(fin,54,SEEK_SET);

    ///padding
    int padding;
    if(latime % 4 != 0)
        padding = 4 - (3 * latime) % 4;
    else
        padding = 0;

    pixel *p;///struct pixel
    p=(pixel*)malloc(latime*inaltime*sizeof(pixel));

    for(i = inaltime-1; i>=0; i--)///liniarizare in vector de pixeli RGB
    {
        for(j = 0; j < latime; j++)
        {
            fread(pRGB, 3, 1, fin);
            p[i*latime+j].R=pRGB[2];
            p[i*latime+j].G=pRGB[1];
            p[i*latime+j].B=pRGB[0];
        }
        fseek(fin,padding,SEEK_CUR);
    }

    p->H=inaltime;
    p->W=latime;

    fclose(fin);
    return p;
}

void chi_squared (char *nume_img)
{
    unsigned int latime,inaltime,i;
    unsigned char pxB,pxG,pxR;
    float red=0,green=0,blue=0,fr;
    FILE *fin;
    fin=fopen(nume_img,"rb");
    exit_if_null(nume_img,fin);

    getSize(fin,&latime,&inaltime);

    pixel *p;
    frecvpx *frecv;
    frecv=(frecvpx*)malloc(256*sizeof(frecvpx));
    memset(frecv,0,256*sizeof(frecvpx));
    p=liniarize(nume_img);
    for (i=0; i<latime*inaltime; i++)
    {
        pxB=p[i].B;
        pxG=p[i].G;
        pxR=p[i].R;
        frecv[pxB].fB++;
        frecv[pxG].fG++;
        frecv[pxR].fR++;
    }
    fr=((float)(latime*inaltime))/256;
    for (i=0; i<256; i++)
    {
        red+=(frecv[i].fR-fr)*(frecv[i].fR-fr)/fr;
        green+=(frecv[i].fG-fr)*(frecv[i].fG-fr)/fr;
        blue+=(frecv[i].fB-fr)*(frecv[i].fB-fr)/fr;
    }

    printf("\nChi-squared %s\n",nume_img);
    printf("R:%.2f\nG:%.2f\nB:%.2f\n ",red,green,blue);
}
void LinToBMP (char *nume_img_test,char *nume_img,pixel *p)///img in care scriu ,img header, vect liniarizat
{
    FILE *fin,*fout;
    unsigned int latime,inaltime;
    int i,j;
    unsigned char c;

    fin= fopen(nume_img,"rb");
    exit_if_null(nume_img,fin);
    fout = fopen(nume_img_test, "wb");

    copyHeader(nume_img,nume_img_test);

    ///header - dimensiuni
    getSize(fin,&latime,&inaltime);
    fseek(fin,54,SEEK_SET);

    int padding;
    if(latime % 4 != 0)
        padding = 4 - (3 * latime) % 4;
    else
        padding = 0;

    fseek(fout, 54, SEEK_SET);
    for(i = inaltime-1; i>=0; i--)
    {
        for(j = 0; j < latime; j++)
        {
            fwrite(&p[i*latime+j].B, 1, 1, fout);
            fwrite(&p[i*latime+j].G, 1, 1, fout);
            fwrite(&p[i*latime+j].R, 1, 1, fout);
            fflush(fout);
        }
        if (i!=0)
            fseek(fout,padding,SEEK_CUR);
    }

    for (i=0; i<padding; i++)
        fwrite(&c,1,1,fout);///completare padd ramas

    fclose(fin);
    fclose(fout);
}

void criptare(char *nume_img, char *enc_img,char *secret_key)///cale init, cale img criptata,cale textfile=key
{
    FILE *fin,*fkey,*fenc;
    unsigned int *random,latime,inaltime,key1,key2,*perm,r;
    int i,poz,x;
    pixel *p,*p_interm,*enc;

    fin= fopen(nume_img,"rb");
    fkey=fopen(secret_key,"r");
    fenc=fopen(enc_img,"wb");

    exit_if_null(nume_img,fin);
    exit_if_null(secret_key,fkey);

    fscanf(fkey,"%u %u",&key1,&key2);

    getSize(fin,&latime,&inaltime);
    random=XORSHIFT32(2*inaltime*latime,key1);///generare 2*W*H nr random
    perm=Durst(inaltime*latime,random);///permutare W*H nr folosing primele W*H nr din random

    p_interm=(pixel*)malloc(latime*inaltime*sizeof(pixel));///img de permutat
    enc=(pixel*)malloc(latime*inaltime*sizeof(pixel));///de criptat

    p=liniarize(nume_img);///pixeli img initiala

    for (i=0; i<inaltime*latime; i++)///permutare
    {
        poz=perm[i];
        p_interm[poz].R=p[i].R;
        p_interm[poz].G=p[i].G;
        p_interm[poz].B=p[i].B;
    }

    ///caz k==0
    x=inaltime*latime;
    r=random[x];
    enc[0].B = key2 ^ p_interm[0].B ^ r;
    r=r>>8;
    key2=key2>>8;
    enc[0].G = key2 ^ p_interm[0].G ^ r;
    r=r>>8;
    key2=key2>>8;
    enc[0].R = key2 ^ p_interm[0].R ^ r;

    ///caz k>0
    for (i=1; i<latime*inaltime; i++)
    {
        x++;
        r=random[x];
        enc[i].B= enc[i-1].B ^ p_interm[i].B ^ r;
        r=r>>8;
        enc[i].G= enc[i-1].G ^ p_interm[i].G ^ r;
        r=r>>8;
        enc[i].R= enc[i-1].R ^ p_interm[i].R ^ r;
    }
    LinToBMP(enc_img,nume_img,enc);

    fclose(fin);
    fclose(fenc);
    fclose(fkey);

    free(random);
    free(perm);
    free(p_interm);
    free(enc);
    free(p);
}

void decriptare(char *enc_img , char *dec_img,char *secret_key)
{
    FILE *fin,*fout,*fkey;
    unsigned int latime,inaltime,key1,key2,x,r,i,poz;
    unsigned int *random,*perm,*inv;

    fin= fopen(enc_img,"rb");
    fkey=fopen(secret_key,"r");
    fout = fopen(dec_img, "wb");

    exit_if_null(enc_img,fin);
    exit_if_null(secret_key,fkey);
    fscanf(fkey,"%u %u",&key1,&key2);

    getSize(fin,&latime,&inaltime);
    random=XORSHIFT32(2*inaltime*latime,key1);///generare 2*W*H nr random
    perm=Durst(inaltime*latime,random);///permutare primele W*H nr din random

    pixel *c_interm,*dec,*enc;
    c_interm=(pixel*)malloc(latime*inaltime*sizeof(pixel));///img de permutat
    dec=(pixel*)malloc(latime*inaltime*sizeof(pixel));///de decriptat

    enc=liniarize(enc_img);///pixeli img initiala (criptata)

    inv=perm_inv(perm,latime*inaltime);///permutarea inversa

///caz k=0
    x=inaltime*latime;
    r=random[x];
    c_interm[0].B=key2 ^ enc[0].B ^ r;
    r=r>>8;
    key2=key2>>8;
    c_interm[0].G=key2 ^ enc[0].G ^ r;
    r=r>>8;
    key2=key2>>8;
    c_interm[0].R=key2 ^ enc[0].R ^ r;

    ///caz k>0
    for (i=1; i<latime*inaltime; i++)
    {
        x++;
        r=random[x];
        c_interm[i].B=enc[i-1].B ^ enc[i].B ^ r;
        r=r>>8;
        c_interm[i].G=enc[i-1].G ^ enc[i].G ^ r;
        r=r>>8;
        c_interm[i].R=enc[i-1].R ^ enc[i].R ^ r;
    }
    for (i=0; i<inaltime*latime; i++)///permutare
    {
        poz=inv[i];
        dec[poz].R=c_interm[i].R;
        dec[poz].G=c_interm[i].G;
        dec[poz].B=c_interm[i].B;
    }

    LinToBMP(dec_img,enc_img,dec);

    fclose(fin);
    fclose(fout);
    fclose(fkey);

    free(dec);
    free(inv);
    free(random);
    free(perm);
    free(c_interm);
    free(enc);
}
pixel colors(char *nume_temp)
{
    pixel p;
    if (strcmp(nume_temp,"cifra0.bmp")==0)
    {
        p.R=255;
        p.G=0;
        p.B=0;
    }
    if (strcmp(nume_temp,"cifra1.bmp")==0)
    {
        p.R=255;
        p.G=255;
        p.B=0;
    }
    if (strcmp(nume_temp,"cifra2.bmp")==0)
    {
        p.R=0;
        p.G=255;
        p.B=0;
    }
    if (strcmp(nume_temp,"cifra3.bmp")==0)
    {
        p.R=0;
        p.G=255;
        p.B=255;
    }
    if (strcmp(nume_temp,"cifra4.bmp")==0)
    {
        p.R=255;
        p.G=0;
        p.B=255;
    }
    if (strcmp(nume_temp,"cifra5.bmp")==0)
    {
        p.R=0;
        p.G=0;
        p.B=255;
    }
    if (strcmp(nume_temp,"cifra6.bmp")==0)
    {
        p.R=192;
        p.G=192;
        p.B=192;
    }
    if (strcmp(nume_temp,"cifra7.bmp")==0)
    {
        p.R=255;
        p.G=140;
        p.B=0;
    }
    if (strcmp(nume_temp,"cifra8.bmp")==0)
    {
        p.R=128;
        p.G=0;
        p.B=128;
    }
    if (strcmp(nume_temp,"cifra9.bmp")==0)
    {
        p.R=128;
        p.G=0;
        p.B=0;
    }
    return p;
}
void chenar(char *nume_temp,char *nume_img,char *img_matches ,corel *c, unsigned int size)
{
    FILE *fin,*ft;
    unsigned int latime,inaltime,x,y,k,i;
    pixel *p_color,p;

    fin=fopen(nume_img,"rb");
    ft=fopen(nume_temp,"rb");

    p_color=liniarize(nume_img);
    getSize(ft,&latime,&inaltime);///dim sablon

    for (k=0; k<size; k++)
    {
        x=c[k].idxL;
        y=c[k].idxC;
        p=colors(c[k].nume);///culoare in functie de sablon
        if (c[k].corr!=-1)
        {
            for (i=y; i<y+latime; i++)///sus
            {
                p_color[x* p_color->W +i].R=p.R;
                p_color[x* p_color->W +i].G=p.G;
                p_color[x* p_color->W +i].B=p.B;
            }

            for (i=x+1; i<x+inaltime; i++)
            {
                p_color[i* p_color->W+y].R=p.R;///stanga
                p_color[i* p_color->W+y].G=p.G;
                p_color[i* p_color->W+y].B=p.B;

                p_color[i*p_color->W + y+latime-1].R=p.R;///dreapta
                p_color[i*p_color->W + y+latime-1].G=p.G;
                p_color[i*p_color->W + y+latime-1].B=p.B;
            }

            for (i=y; i<y+latime; i++)
            {
                p_color[(x+inaltime-1)*p_color->W+i].R=p.R;///jos
                p_color[(x+inaltime-1)*p_color->W+i].G=p.G;
                p_color[(x+inaltime-1)*p_color->W+i].B=p.B;
            }
        }
    }
    LinToBMP(img_matches,nume_img,p_color);
    fclose(fin);
    fclose(ft);
    free(p_color);
}
float medie (pixel *p_img, pixel *p_temp,int x,int y)///medie fereastra
{
    int i,j;
    float sum=0;
    for (i=x; i<x+ p_temp->H ; i++)
    {
        for (j=y; j<y+ p_temp->W ; j++)
        {
            sum+=p_img[i* p_img->W +j].R;
        }
    }
    return sum/(p_temp->W * p_temp->H);
}
float deviatie(pixel *p_img, pixel *p_temp,int x,int y,float med)
{
    int i,j;
    float dev=0,n;
    for (i=x; i<x+ p_temp->H ; i++)
    {
        for (j=y; j<y+ p_temp->W ; j++)
        {
            dev+=(p_img[i* p_img->W +j].R - med)*(p_img[i* p_img->W +j].R - med);
        }
    }
    n=p_temp->H * p_temp->W-1;
    dev=dev/n;
    dev=sqrt(dev);
    return dev;
}
void corelation (pixel *p_img, char *nume_temp, float prag,corel *c, unsigned int *cnt_cor)
{
    FILE *ftemp;
    unsigned int latime,inaltime,x,y,i,j;
    unsigned int latimeT,inaltimeT;
    float med_T,med_F,dev_T,dev_F,n,corr=0;
    pixel *p_temp;

    ftemp=fopen(nume_temp,"rb");///deschid sablonul

    p_temp=liniarize(nume_temp);

    latime=p_img->W;
    inaltime=p_img->H;
    latimeT=p_temp->W;
    inaltimeT=p_temp->H;
    n=latimeT*inaltimeT;

    med_T=medie(p_temp,p_temp,0,0);///calc medie template
    dev_T=deviatie(p_temp,p_temp,0,0,med_T);///calc deviatie template

    for (x=0; x<inaltime - inaltimeT; x++)///parcurg imagine
    {
        for (y=0; y<latime - latimeT; y++)
        {
            med_F=medie(p_img,p_temp,x,y);///calc medie fereastra
            dev_F=deviatie(p_img,p_temp,x,y,med_F);///calc deviatie fereastra
            corr=0;
            for (i=0; i<inaltimeT; i++)///parcurg template
            {
                for (j=0; j<latimeT; j++)
                {
                    corr+=(p_img[(x+i)*latime+(y+j)].R - med_F) * (p_temp[i*latimeT+j].R - med_T)/(dev_T * dev_F);
                }
            }
            corr=corr/n;
            if (corr>prag)
            {
                c[*cnt_cor].nume=(char*)malloc(30);
                c[*cnt_cor].idxL=x;
                c[*cnt_cor].idxC=y;
                strcpy(c[*cnt_cor].nume,nume_temp);
                c[*cnt_cor].corr=corr;
                (*cnt_cor)++;
            }
        }
    }
    free(p_temp);
    fclose(ftemp);
}
int comp (const void *a, const void *b)
{
    corel *c1 = (corel*)a;
    corel *c2 = (corel*)b;

    if (c1->corr < c2->corr)
        return 1;
    else return -1;
    return 0;
}
float suprapunere(corel A, corel B,int latime,int inaltime)
{
    int st1,st2,sus1,sus2;
    int dr1,dr2,jos1,jos2;
    int iSt,iDr,iSus,iJos;
    int arieint;
    float supr;
    st1=A.idxL;
    st2=B.idxL;
    sus1=A.idxC;
    sus2=B.idxC;

    dr1=st1+inaltime;
    dr2=st2+inaltime;
    jos1=sus1+latime;
    jos2=sus2+latime;

    iSt=max(st1,st2);
    iDr=min(dr1,dr2);
    iJos=min(jos1,jos2);
    iSus=max(sus1,sus2);

    arieint=(iDr-iSt) * (iJos-iSus);
    if (iSt<iDr && iSus<iJos)
    {
        supr=(1.0*arieint)/(2.0*latime*inaltime-arieint);
        return supr;
    }
    return -1;
}

void eliminare(corel *c, char *nume_sablon,int cnt_cor)
{
    FILE *ft;
    unsigned int latime,inaltime,i,j;
    float supr;
    ft=fopen(nume_sablon,"rb");
    exit_if_null(nume_sablon,ft);
    getSize(ft,&latime,&inaltime);
    for (i=0; i<cnt_cor-1; i++)
        for (j=i+1; j<cnt_cor; j++)
        {
            supr=suprapunere(c[i],c[j],latime,inaltime);
            if (supr>0.2)
            {
                if (c[i].corr > c[j].corr)
                {
                    c[j].corr=-1;
                }
                else
                {
                    c[i].corr=-1;
                    break;
                }
            }
        }
}
void template_matching(char *nume_TM,char *img_matches,char *templates)
{
    FILE *ftemp,*fgray;
    unsigned int i,cnt_cor=0;
    char *nume_sablon,*gray_img;
    float prag=0.5;
    pixel *p_gray;
    corel *c;

    gray_img=(char*)malloc(30);
    strcpy(gray_img,"gray_img.bmp");
    nume_sablon=(char*)malloc(30);
    grayscale_image(nume_TM,gray_img);

    ftemp=fopen(templates,"r");
    fgray=fopen(gray_img,"rb");
    exit_if_null(templates,ftemp);
    exit_if_null(gray_img,fgray);

    p_gray=liniarize(gray_img);
    c=(corel*)malloc(p_gray->H * p_gray->W *sizeof(corel));

    while(fgets(nume_sablon,50,ftemp)!=NULL)///citeste fiecare sablon din fisier
    {
        elim_endl(&nume_sablon);
        corelation(p_gray, nume_sablon,prag,c,&cnt_cor);///salvez corelatia pt fiecare sablon in vectorul c
    }
    //printf("cnt =  %u\n",cnt_cor);
    qsort(c,cnt_cor,sizeof(corel),comp);///sortez descrescator dupa corelatie
    eliminare(c,nume_sablon,cnt_cor);///elim detectiile care se suprapun
    chenar(nume_sablon,nume_TM,img_matches,c,cnt_cor);///desenez detectiile din vector

    fclose(fgray);
    fclose(ftemp);
    free(nume_sablon);
    free(c);
    free(p_gray);
}
int main()
{
    char *nume_img, *nume_img_enc, *secret_key, *nume_img_dec,*nume_TM,*templates,*img_matches;

    citire(&nume_img,&nume_img_enc,&nume_img_dec,&secret_key,&nume_TM,&templates,&img_matches);

    criptare(nume_img,nume_img_enc,secret_key);
    decriptare(nume_img_enc,nume_img_dec,secret_key);

    chi_squared(nume_img);
    chi_squared(nume_img_enc);

    ///imagine + img in care desenez + fisier in care se afla caile templateurilor
    template_matching(nume_TM,img_matches,templates);
    free(nume_img);
    free(nume_img_enc);
    free(secret_key);
    free(nume_img_dec);
    free(nume_TM);
    free(templates);
    free(img_matches);
    return 0;
}
