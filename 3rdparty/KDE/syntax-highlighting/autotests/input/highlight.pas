PROGRAM MKFILE;

USES    crt,algebra;

TYPE    dt = record e : record name     : string [15];
                               zeichen  : string [2];
                               ozahl    : byte;
                               amasse   : string [9];
                               ent      : array [1..4]  of string [70]
                        end;

                    c : record txt      : array [1..10] of string [40];
                               radien   : array [1..2]  of string [6];
                               elneg    : string [7];
                               kladung  : string [5]
                        end;

                    p : record smt      : string [8];
                               sdt      : string [8];
                               dichte   : string [15];
                               volmol   : string [13];
                        end;

                    u : record bio      : array [1..13] of string [30];
                               hauf     : array [1..4] of string [30];
                               geo      : array [1..6] of string [30];
                        end;

                    s : record n        : byte;
                               bele     : array [1..7] of byte;
                        end;

                    i : record first    : word;
                               last     : word;
                               c1       : array [1..100] of byte;
                               c2       : array [1..100] of byte;
                               c3       : array [1..100] of byte;
                               c4       : array [1..100] of byte;
                               hzeit    : array [1..100] of array [1..2] of string [7];
                        end;
             end;

        cfg = record pse  : string;
                     bgi  : string;
                     info : string;
              end;


VAR     i     : dt;
        h     : file of dt;

        t     : cfg;
        u     : file of cfg;

        zstr  : string;
        ozahl : byte;

PROCEDURE MKELEMENT (ozahl:byte);
BEGIN     CASE ozahl OF
             1    : i.e.name:='Wasserstoff';
             2    : i.e.name:='Helium';
             3    : i.e.name:='Lithium';
             4    : i.e.name:='Beryllium';
             5    : i.e.name:='Bor';
             6    : i.e.name:='Kohlenstoff';
             7    : i.e.name:='Stickstoff';
             8    : i.e.name:='Sauerstoff';
             9    : i.e.name:='Fluor';
             10   : i.e.name:='Neon';
             11   : i.e.name:='Natrium';
             12   : i.e.name:='Magnesium';
             13   : i.e.name:='Aluminium';
             14   : i.e.name:='Silicium';
             15   : i.e.name:='Phosphor';
             16   : i.e.name:='Schwefel';
             17   : i.e.name:='Chlor';
             18   : i.e.name:='Argon';
             19   : i.e.name:='Kalium';
             20   : i.e.name:='Calzium';
             21   : i.e.name:='Scandium';
             22   : i.e.name:='Titan';
             23   : i.e.name:='Vanadium';
             24   : i.e.name:='Chrom';
             25   : i.e.name:='Mangan';
             26   : i.e.name:='Eisen';
             27   : i.e.name:='Cobalt';
             28   : i.e.name:='Nickel';
             29   : i.e.name:='Kupfer';
             30   : i.e.name:='Zink';
             31   : i.e.name:='Gallium';
             32   : i.e.name:='Germanium';
             33   : i.e.name:='Arsen';
             34   : i.e.name:='Selen';
             35   : i.e.name:='Brom';
             36   : i.e.name:='Krypton';
             37   : i.e.name:='Rubidium';
             38   : i.e.name:='Strontium';
             39   : i.e.name:='Yttrium';
             40   : i.e.name:='Zirconium';
             41   : i.e.name:='Niob';
             42   : i.e.name:='Molybd�n';
             43   : i.e.name:='Technetium';
             44   : i.e.name:='Ruthenium';
             45   : i.e.name:='Rhodium';
             46   : i.e.name:='Paladium';
             47   : i.e.name:='Silber';
             48   : i.e.name:='Cadmium';
             49   : i.e.name:='Indium';
             50   : i.e.name:='Zinn';
             51   : i.e.name:='Antimon';
             52   : i.e.name:='Tellur';
             53   : i.e.name:='Iod';
             54   : i.e.name:='Xenon';
             55   : i.e.name:='Caesium';
             56   : i.e.name:='Barium';
             57   : i.e.name:='Lanthan';
             58   : i.e.name:='Cer';
             59   : i.e.name:='Praseodym';
             60   : i.e.name:='Neodym';
             61   : i.e.name:='Promelthium';
             62   : i.e.name:='Samarium';
             63   : i.e.name:='Europium';
             64   : i.e.name:='Gadolinium';
             65   : i.e.name:='Terbium';
             66   : i.e.name:='Dysprosium';
             67   : i.e.name:='Holmiuim';
             68   : i.e.name:='Erbium';
             69   : i.e.name:='Thulium';
             70   : i.e.name:='Ytterbium';
             71   : i.e.name:='Lutetium';
             72   : i.e.name:='Hafnium';
             73   : i.e.name:='Tanal';
             74   : i.e.name:='Wolfram';
             75   : i.e.name:='Rhenium';
             76   : i.e.name:='Osmium';
             77   : i.e.name:='Iridium';
             78   : i.e.name:='Platin';
             79   : i.e.name:='Gold';
             80   : i.e.name:='Quecksilber';
             81   : i.e.name:='Thalium';
             82   : i.e.name:='Blei';
             83   : i.e.name:='Wissmut';
             84   : i.e.name:='Polonium';
             85   : i.e.name:='Astat';
             86   : i.e.name:='Radon';
             87   : i.e.name:='Francium';
             88   : i.e.name:='Radium';
             89   : i.e.name:='Actinium';
             90   : i.e.name:='Thorium';
             91   : i.e.name:='Protactinium';
             92   : i.e.name:='Uran';
             93   : i.e.name:='Neptunium';
             94   : i.e.name:='Plutonium';
             95   : i.e.name:='Americum';
             96   : i.e.name:='Curium';
             97   : i.e.name:='Berkelium';
             98   : i.e.name:='Californium';
             99   : i.e.name:='Einsteinium';
             100  : i.e.name:='Fermium';
             101  : i.e.name:='Mendelevium';
             102  : i.e.name:='Nobelium';
             103  : i.e.name:='Lawrencium';
             104  : i.e.name:='Rutherfordium';
             105  : i.e.name:='Dubnium';
          END;

          CASE ozahl OF
             1    : i.e.zeichen:='H';
             2    : i.e.zeichen:='He';
             3    : i.e.zeichen:='Li';
             4    : i.e.zeichen:='Be';
             5    : i.e.zeichen:='B';
             6    : i.e.zeichen:='C';
             7    : i.e.zeichen:='N';
             8    : i.e.zeichen:='O';
             9    : i.e.zeichen:='F';
             10   : i.e.zeichen:='Ne';
             11   : i.e.zeichen:='Na';
             12   : i.e.zeichen:='Mg';
             13   : i.e.zeichen:='Al';
             14   : i.e.zeichen:='Si';
             15   : i.e.zeichen:='P';
             16   : i.e.zeichen:='S';
             17   : i.e.zeichen:='Cl';
             18   : i.e.zeichen:='Ar';
             19   : i.e.zeichen:='K';
             20   : i.e.zeichen:='Sa';
             21   : i.e.zeichen:='Sc';
             22   : i.e.zeichen:='Ti';
             23   : i.e.zeichen:='V';
             24   : i.e.zeichen:='Cr';
             25   : i.e.zeichen:='Mn';
             26   : i.e.zeichen:='Fe';
             27   : i.e.zeichen:='Co';
             28   : i.e.zeichen:='Ni';
             29   : i.e.zeichen:='Cu';
             30   : i.e.zeichen:='Zn';
             31   : i.e.zeichen:='Ga';
             32   : i.e.zeichen:='Ge';
             33   : i.e.zeichen:='As';
             34   : i.e.zeichen:='Se';
             35   : i.e.zeichen:='Br';
             36   : i.e.zeichen:='Kr';
             37   : i.e.zeichen:='Rb';
             38   : i.e.zeichen:='Sr';
             39   : i.e.zeichen:='Y';
             40   : i.e.zeichen:='Zr';
             41   : i.e.zeichen:='Nb';
             42   : i.e.zeichen:='Mo';
             43   : i.e.zeichen:='Tc';
             44   : i.e.zeichen:='Ru';
             45   : i.e.zeichen:='Rh';
             46   : i.e.zeichen:='Pd';
             47   : i.e.zeichen:='Ag';
             48   : i.e.zeichen:='Cd';
             49   : i.e.zeichen:='In';
             50   : i.e.zeichen:='Sn';
             51   : i.e.zeichen:='Sb';
             52   : i.e.zeichen:='Te';
             53   : i.e.zeichen:='I';
             54   : i.e.zeichen:='Xe';
             55   : i.e.zeichen:='Cs';
             56   : i.e.zeichen:='Ba';
             57   : i.e.zeichen:='La';
             58   : i.e.zeichen:='Ce';
             59   : i.e.zeichen:='Pr';
             60   : i.e.zeichen:='Nd';
             61   : i.e.zeichen:='Pm';
             62   : i.e.zeichen:='Sm';
             63   : i.e.zeichen:='Eu';
             64   : i.e.zeichen:='Gd';
             65   : i.e.zeichen:='Tb';
             66   : i.e.zeichen:='Dy';
             67   : i.e.zeichen:='Ho';
             68   : i.e.zeichen:='Er';
             69   : i.e.zeichen:='Tm';
             70   : i.e.zeichen:='Yb';
             71   : i.e.zeichen:='Lu';
             72   : i.e.zeichen:='Hf';
             73   : i.e.zeichen:='Ta';
             74   : i.e.zeichen:='W';
             75   : i.e.zeichen:='Re';
             76   : i.e.zeichen:='Os';
             77   : i.e.zeichen:='Ir';
             78   : i.e.zeichen:='Pt';
             79   : i.e.zeichen:='Au';
             80   : i.e.zeichen:='Hg';
             81   : i.e.zeichen:='Tl';
             82   : i.e.zeichen:='Pb';
             83   : i.e.zeichen:='Bi';
             84   : i.e.zeichen:='Po';
             85   : i.e.zeichen:='At';
             86   : i.e.zeichen:='Rn';
             87   : i.e.zeichen:='Fr';
             88   : i.e.zeichen:='Ra';
             89   : i.e.zeichen:='Ac';
             90   : i.e.zeichen:='Th';
             91   : i.e.zeichen:='Pa';
             92   : i.e.zeichen:='U';
             93   : i.e.zeichen:='Np';
             94   : i.e.zeichen:='Pu';
             95   : i.e.zeichen:='Am';
             96   : i.e.zeichen:='Cm';
             97   : i.e.zeichen:='Bk';
             98   : i.e.zeichen:='Cf';
             99   : i.e.zeichen:='Es';
             100  : i.e.zeichen:='Fm';
             101  : i.e.zeichen:='Md';
             102  : i.e.zeichen:='No';
             103  : i.e.zeichen:='Lr';
             104  : i.e.zeichen:='Rf';
             105  : i.e.zeichen:='Db';
          END;
END;

PROCEDURE MKBELE (ozahl:byte);
BEGIN     IF (ozahl>2) THEN i.s.bele[1]:=2;

          IF (ozahl>9) THEN i.s.bele[2]:=8;

          IF (ozahl>28) THEN i.s.bele[3]:=18;

          IF (ozahl>69) THEN i.s.bele[4]:=32;

          IF (ozahl>87) THEN i.s.bele[7]:=2;

          IF (ozahl>0) and (ozahl<3) THEN BEGIN i.s.n:=1;
                                                i.s.bele[1]:=ozahl;
                                          END;

          IF (ozahl>2) and (ozahl<11) THEN BEGIN i.s.n:=2;
                                                 i.s.bele[2]:=ozahl-2;
                                           END;

          IF (ozahl>10) and (ozahl<19) THEN BEGIN i.s.n:=3;
                                                  i.s.bele[3]:=ozahl-10;
                                            END;

          IF (ozahl>18) and (ozahl<21) THEN BEGIN i.s.n:=4;
                                                  i.s.bele[3]:=8;
                                                  i.s.bele[4]:=ozahl-18;
                                            END;

          IF (ozahl>20) and (ozahl<24) THEN BEGIN i.s.n:=4;
                                                  i.s.bele[3]:=ozahl-12;
                                                  i.s.bele[4]:=2;
                                            END;

          IF (ozahl=24) THEN BEGIN i.s.n:=4;
                                   i.s.bele[3]:=13;
                                   i.s.bele[4]:=1;
                             END;

          IF (ozahl>24) and (ozahl<29) THEN BEGIN i.s.n:=4;
                                                  i.s.bele[3]:=ozahl-12;
                                                  i.s.bele[4]:=2;
                                            END;

          IF (ozahl>28) and (ozahl<37) THEN BEGIN i.s.n:=4;
                                                  i.s.bele[4]:=ozahl-28;
                                            END;

          IF (ozahl>36) and (ozahl<39) THEN BEGIN i.s.n:=5;
                                                  i.s.bele[4]:=8;
                                                  i.s.bele[5]:=ozahl-36;
                                            END;

          IF (ozahl>38) and (ozahl<41) THEN BEGIN i.s.n:=5;
                                                  i.s.bele[4]:=ozahl-30;
                                                  i.s.bele[5]:=2;
                                            END;

          IF (ozahl>40) and (ozahl<46) THEN BEGIN i.s.n:=5;
                                                  i.s.bele[4]:=ozahl-29;
                                                  i.s.bele[5]:=1;
                                            END;

          IF (ozahl=46) THEN BEGIN i.s.n:=4;
                                   i.s.bele[4]:=18;
                             END;

          IF (ozahl>46) and (ozahl<55) THEN BEGIN i.s.n:=5;
                                                  i.s.bele[4]:=18;
                                                  i.s.bele[5]:=ozahl-46;
                                            END;

          IF (ozahl>54) and (ozahl<57) THEN BEGIN i.s.n:=6;
                                                  i.s.bele[4]:=18;
                                                  i.s.bele[5]:=8;
                                                  i.s.bele[6]:=ozahl-54;
                                            END;

          IF (ozahl=57) or (ozahl=64) THEN BEGIN i.s.n:=6;
                                                 i.s.bele[4]:=ozahl-39;
                                                 i.s.bele[5]:=9;
                                                 i.s.bele[6]:=2;
                                           END;

          IF ((ozahl>57) and (ozahl<64)) or ((ozahl>64) and (ozahl<71))
                                          THEN BEGIN i.s.n:=6;
                                                     i.s.bele[4]:=ozahl-38;
                                                     i.s.bele[5]:=8;
                                                     i.s.bele[6]:=2;
                                               END;

          IF (ozahl=64) THEN BEGIN i.s.n:=6;
                                   i.s.bele[4]:=25;
                                   i.s.bele[5]:=9;
                                   i.s.bele[6]:=2;
                             END;

          IF (ozahl>70) and (ozahl<78) THEN BEGIN i.s.n:=6;
                                                  i.s.bele[5]:=ozahl-62;
                                                  i.s.bele[6]:=2;
                                            END;

          IF (ozahl>77) and (ozahl<80) THEN BEGIN i.s.n:=6;
                                                  i.s.bele[5]:=ozahl-61;
                                                  i.s.bele[6]:=1;
                                            END;

          IF (ozahl>79) and (ozahl<87) THEN BEGIN i.s.n:=6;
                                                  i.s.bele[5]:=18;
                                                  i.s.bele[6]:=ozahl-78;
                                            END;

          IF (ozahl>86) and (ozahl<89) THEN BEGIN i.s.n:=7;
                                                  i.s.bele[5]:=18;
                                                  i.s.bele[6]:=8;
                                                  i.s.bele[7]:=ozahl-86;
                                            END;

          IF (ozahl>88) and (ozahl<91) THEN BEGIN i.s.n:=7;
                                                  i.s.bele[5]:=18;
                                                  i.s.bele[6]:=ozahl-80;
                                            END;

          IF ((ozahl>90) and (ozahl<94)) or (ozahl=96)
                                       THEN BEGIN i.s.n:=7;
                                                  i.s.bele[5]:=ozahl-71;
                                                  i.s.bele[6]:=9;
                                            END;

          IF (ozahl>93) and (ozahl<96) THEN BEGIN i.s.n:=7;
                                                  i.s.bele[5]:=ozahl-70;
                                                  i.s.bele[6]:=8;
                                            END;

          IF (ozahl>96) and (ozahl<103) THEN BEGIN i.s.n:=7;
                                                   i.s.bele[5]:=ozahl-70;
                                                   i.s.bele[6]:=8;
                                             END;

         IF (ozahl>102) and (ozahl<106) THEN BEGIN i.s.n:=7;
                                                   i.s.bele[5]:=32;
                                                   i.s.bele[6]:=ozahl-94;
                                             END;
END;

BEGIN     Assign (u,'install.cfg');
          Reset (u);
          Read (u,t);
          Close (u);

          clrscr;

          FOR ozahl:=1 TO 105 DO
              BEGIN INTSTR(ozahl,zstr);

                    assign (h,t.info + zstr + '.pse');
                    write (t.info + zstr + '.pse');
                    rewrite (h);
                    reset (h);

                    i.e.ozahl:=ozahl;
                    MKELEMENT(ozahl);
                    MKBELE(ozahl);

                    write (h,i);
                    close (h);

                    Writeln ('.......Erstellt');
              END;
              readkey;
END.
