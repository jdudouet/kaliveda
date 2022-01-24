c$Id: faire_arbre_c2.f,v 1.5 2006/05/02 17:36:59 franklan Exp $                                                                   
c---------------------------------------------------------------------  
c     Ici se trouvent reunies les diverses routines personnelles a    | 
c     l'utilisateur. Celui trouvera donc la routine INI_HBOOK,        | 
c     appelee en tout debut de programme et servant a initialiser     | 
c     les fichiers type Hbook par exemple. La routine INI_RUN est     | 
c     appelee a chaque fois que commence la lecture d'un run DST.     | 
c     La routine TRAITEMENT est appelee a CHAQUE evenement et permet  | 
c     de realiser l'analyse des donnees recuperees via les COMMONs    | 
c     definis dans VEDA_6 INCL ( cf fichier VEDA_6 INCL ).            | 
c     La routine FIN_RUN est appelee a la fin de la lecture d'un      | 
c     run DST et la routine CLOSE_HB est appelee en toute fin de      | 
c     lecture.                                                        | 
c                                                                     | 
c                                                                     | 
c Version du 12/12/94                                                 | 
c --------------------------------------------------------------------  
c                                                                       
      subroutine INI_HBOOK                                              
                                                                        
      return                                                            
      end                                                               
                                                                        
c---------------------------------------------------------------------  
c --- Routine appelee evt par evt, les caracteristiques de l'evt        
c --- sont dans veda_6 INCL                                             
c---------------------------------------------------------------------  
c --- Compteurs disponibles sur la lecture des cartouches disponibles   
c --- via VEDA_REL INCL :                                               
c                                                                       
c   - Nbloclus  : Nbre de blocs lus dans le fichier courant             
c   - Nbloc_tot : Nbre TOTAL de blocs lus (cumul de Nbloclus)           
c   - Nevtlus   : Nbre d'evenements lus dans le fichier courant         
c   - Nevt_tot  : Nbre total d'evenement lus (cumul de Nevtlus)         
c                                                                       
c---------------------------------------------------------------------  
c --- Variables logiques disponibles concernant la lecture :            
c                                                                       
c   - Fin_lect_run : valeur a .False. pour la lecture des evenements    
c                     du fichier courant et a .True. lorsqu'on ne       
c                     veut plus lire d'evt dans le run DST courant.     
c                                                                       
c   - Fin_lect_gen : valeur a .False. pour la lecture des evenements    
c                     de tous les fichiers courant et a .True. lors-    
c                     qu'on ne veut plus lire du tout d'evt .           
c                                                                       
c   Ces 2 variables sont a .False. lorsqu'on demarre la lecture et      
c   peuvent etre passees a .True. lorsqu'on desire ne plus lire         
c   d'evenements soit dans le run courant ( on reprend alors la         
c   lecture des evts au run suivant s'il existe) soit pour le           
c   job ( on arrete alors proprement le batch ).                        
c                                                                       
c---------------------------------------------------------------------  
      subroutine TRAITEMENT                                             
                                                                        
      include 'veda_rel.incl'                                           
      include 'veda_5.incl'                                             
      include 'veda_6.incl'                                             
      common /rootwrite/ necrit, nfiles, nrustines, nrustines_camp
		real*4 bragg(100)
		common/bragg/bragg
                                   
		character*60 filename                               
      integer*4  indice(500)
      integer*4  imult,icans
      logical*1  ok                                                                       

	  	call clean_evt() ! 2eme campagne - on nettoie l'evenement des modules douteux
	  
c ---- Inserez ici le traitement evt par evt                            
c      call initevt()
      pi=3.1415927                                                      
      
      if(npart_traite.gt.0) then
      
         call new_event(npart_traite)
         
       	necrit=necrit+1

      	do j=1,npart_traite
				
				!!Specifique a la campagne 2
				!!
				!! Traitement du pic de Bragg
				if(icou(j).ge.2.and.icou(j).le.9.and.code(j).eq.4)then ! Bragg pour C2-9
					if(z(j).ge.80.and.z(j).lt.85)then
						if(de2(j).le.bragg(z(j))*2.) then
							code(j)=5
							nrustines=nrustines+1
							nrustines_camp=nrustines_camp+1
						endif	
					else if(z(j).ge.85)then
						if(de2(j).le.bragg(z(j))*3.) then 
							code(j)=5
							nrustines=nrustines+1
							nrustines_camp=nrustines_camp+1
						endif	
					else
						if(de2(j).le.bragg(z(j))) then
							code(j)=5
							nrustines=nrustines+1
							nrustines_camp=nrustines_camp+1
						endif	
					end if
				else if(icou(j).ge.10.and.icou(j).le.17.and.code(j).eq.4)then ! Bragg pour C10-17
					if(z(j).ge.80)then
						if(de3(j).le.bragg(z(j))*2.) then
							code(j)=5
							nrustines=nrustines+1
							nrustines_camp=nrustines_camp+1
						endif	
					else
						if(de3(j).le.bragg(z(j))) then
							code(j)=5
							nrustines=nrustines+1
							nrustines_camp=nrustines_camp+1
						endif	
					end if
				end if
				!!
				!!Passage des Phoswich en code 2
				if (icou(j).eq.1.and.code(j).eq.4) then
					code(j)=2
					nrustines=nrustines+1
					nrustines_camp=nrustines_camp+1
				endif
				!!
				!!Saturation 
				if(icou(j).ge.2.and.icou(j).le.4.and.code(j).eq.4.and.canal(j,5).gt.3800) then
					code(j)=15
					nrustines=nrustines+1
					nrustines_camp=nrustines_camp+1
				endif
				if(icou(j).ge.6.and.icou(j).le.9.and.code(j).eq.4.and.canal(i,5).eq.4095) then
					code(j)=15
					nrustines=nrustines+1
					nrustines_camp=nrustines_camp+1
				endif
				!!
				!! Fin campgane2 specifique a la campagne 2
				!!
				if (code(j).ge.2.and.code(j).le.11.and.ecode(j).ge.1.and.ecode(j).le.2) then
					if (z(j).eq.0) then
						code(j)=15
						nrustines=nrustines+1
					endif
					if ( ener(j).le.0 ) then
						ener(j)=0
						ecode(j)=14
						code(j)=15
						nrustines=nrustines+1
					endif
					if (z(j).ge.1.and.a(j).eq.0) then
						code(j)=15
						nrustines=nrustines+1
					endif
					if ( icou(j).le.0.or.icou(j).ge.18.or.
     &					imod(j).le.0.or.icou(j).ge.25 ) then
	  					code(j)=15
						nrustines=nrustines+1
	  				endif
				endif
            
            call new_particle(code(j),ecode(j),icou(j),imod(j),z(j),a(j),ener(j),
     &       de1(j),de2(j),de3(j),de4(j),de5(j),mt(j))
				
			enddo
         
         call fill_tree()                                                            
		                                                                        
      endif !if(imult > 0) then

110   format(1x,'BAD: event#',i7,'  particle#',i3,' Z=',i3,' code=',i2)
                                                                        
      return                                                            
      end                                                               
                                                                        
c-------------------------------------------------------------------    
c --- Routine appelee en debut de lecture d'un run                      
c-------------------------------------------------------------------    
      subroutine INI_RUN()                                              
                                                                        
      include 'veda_rel.incl'                                           
      include 'veda_5.incl'                                             
      include 'veda_6.incl'                                             
      include 'veda_11.incl'   
c      include 'anin_1.incl'                                             
      common /rootwrite/ necrit, nfiles, nrustines, nrustines_camp
	  character*1 campaign                                 
	  character*60 filename                                 
                                                                        
c --- Faites ici les initialisations de runs                            
      print *,'Debut du traitement du run ',numerun,'...'               
      
c -- initialize ROOT TTree interface	   
      call init_rfi(numerun)
      
		call pic_de_bragg
      
		necrit=0
      nrustines=0
      nrustines_camp=0
		
      xmnuc=931.5                                                       
      ptot=sqrt((esura+xmnuc)**2-xmnuc**2)*aproj                        
      ztot=zcib+zproj                                                   
      vproj=ptot/(aproj*(esura+xmnuc))                                  
      zvtot=zproj*vproj                                                 
      vcm=aproj*vproj/(aproj+acib)                                      
      ecm=aproj*acib*esura/(aproj+acib)**2  
      iap=aproj                            
      izp=zproj                            
      iac=acib                            
      izc=zcib                            
      print 10,esura,projectile,aproj,zproj,cible,acib,zcib             
      print 20,ptot,vproj,zvtot                                         
      print 30,vcm,ecm
      
		filename=nomjob(1:long_job)//'.info'
		open(unit=87,file=filename,status='new')
		write(87,*) numerun, zproj, aproj, zcib, acib, esura
		close(87)	

      return                                                            
10    format(1x,'E/A=',f10.2,/,1x,'Projectile :',a3,2(1x,f4.0),/,       
     1                         1x,'Cible      :',a3,2(1x,f4.0))         
20    format(1x,'Ptot  :',f15.3,'  Vproj/C :',f6.4,'    ZVproj :',      
     1                                             f15.3)               
30    format(1x,'Vcm/C :',f6.4,'    Ecm disp :',f10.4,' MeV/A.',/)      
40    format(2(1x,i3),3(1x,f10.2),' 0 0. 0 0 0 ',6(1x,'0.'),' 0')
      end                                                               
                                                                        
c-------------------------------------------------------------------    
c --- Routine appelee en fin de lecture d'un run                        
c-------------------------------------------------------------------    
      subroutine FIN_RUN()
                                                                        
      include 'veda_rel.incl'                                           
      include 'veda_5.incl'                                             
      include 'veda_6.incl'                                             
      common /ttime/ ttot,tmin,tmax                                     
      common /rootwrite/ necrit, nfiles, nrustines, nrustines_camp                                
                                                                        
      print *,'Fin de traitement du run ',numerun,'...'                 
      print *,'Evenements traites       :',nevt,'/',necrit
      print *,'Taux d''aceptes           :',necrit*100./nevt                        
      print *,'Temps moyen de traitement:',ttot/float(nevt)             
      print *,'Temps min                :',tmin                         
      print *,'Temps max                :',tmax                         
      print *,'Number of files written :', nfiles                                                       
      print *,' '                                                       

c -- close ROOT file
      call close_rfi()
      
      return                                                            
      end                                                               

c-------------------------------------------------------------------    
c --- Routine appelee en fin de lecture d'une tache                     
c-------------------------------------------------------------------    
      subroutine close_hb                                               
                                                                        
      include 'veda_rel.incl'                                          
      include 'veda_5.incl'                                            
      include 'veda_6.incl'                                            

c --- Inserez ici les fermetures de fichiers (Hbooks, ... )           
                                                                        
                                                                             
      return                                                          
      end                                                              
      
c-------------------------------------------------------------------    
		subroutine pic_de_bragg                                         
      
		integer*4 z,a,long
		real*4 e,de1,de2
		real*4 bragg(100)
		common/bragg/bragg
		character*200 filename,vedadata
		
		print*,'**************************************************'
		print*,' Rustines Camp2 '
		
	  call GETENVF ('VEDA_DATA',vedadata)
		i=0
		long=0
		do while (long.eq.0)
		  i=i+1
		  if(vedadata(i:i).eq.' ')long=i-1
		enddo
		
	  filename=vedadata(1:long)//'bragg_pic.out'
!		filename='bragg_pic.out'	
		open(unit=50,file=filename
     &	,status='old')
	  
10 	read(50,*,err=200,end=100)z,a,e,de1,de2
		bragg(z)=de2
		go to 10
100 	print*,'fin de la lecture du fichier pour pic de Bragg'
		close(50)
		return
200 	print*,'erreur à la lecture Bragg'
		
		return
		end                                        
c-------------------------------------------------------------------    
