	  /  D   k820309    h          14.0        Z��W                                                                                                           
       pef.f90 PEF                      @                             
                            @                             
                            @                             
                         @                                '�                    #FLT    #LAG    #MIS                �                                                            	            &                                                       �                                          H                             &                                                       �                                          �                             &                                           #         @                                                      #X_IN 	   #AA_IN 
                                            	                   	               &                                                                                      
     �               #FILTER    #         @                                                     #SOLVER_SMP%SIZE    #SOLVER_SMP%PRESENT    #SOLVER_SMP%DOT_PRODUCT    #M    #D    #FOP    #STEPPER    #NITER    #WOP    #JOP "   #M0 '   #ERR (   #RESD )   #MMOV *   #RMOV +   #VERB ,                                                    SIZE                                                  PRESENT                                                  DOT_PRODUCT                                                              	               &                                                     
                                                    	              &                                           %         @                                                           #ADJ    #ADD    #M    #D              
                                                      
                                                                                                         	               &                                                                                                        	               &                                           %         @                                                           #FORGET    #M    #G    #RR    #GG                                                                                                                       	               &                                                                                                        	               &                                                                                                        	 	              &                                                                                                        	 
              &                                                     
                                             %         @                                                          #ADJ    #ADD    #M     #D !             
                                                      
                                                                                                          	               &                                                                                     !                   	               &                                           %         @                               "                           #ADJ #   #ADD $   #M %   #D &             
                                 #                     
                                 $                                                     %                   	               &                                                                                     &                   	               &                                                     
                                 '                   	              &                                                                                      (                   	               &                                                                                      )                   	               &                                                                                     *                   	               &                   &                                                                                     +                   	               &                   &                                                     
                                 ,           %         @   @                            -                           #ADJ .   #ADD /   #A 0   #Y 1             
                                  .                     
                                  /                                                      0                   	               &                                                                                      1                   	               &                                           %         @  @                            2                          #CGSTEP%ALLOCATED 3   #CGSTEP%SIZE 4   #CGSTEP%DOT_PRODUCT 5   #CGSTEP%SUM 6   #CGSTEP%DPROD 7   #CGSTEP%MAX 8   #FORGET 9   #X :   #G ;   #RR <   #GG =                                               3     ALLOCATED                                             4     SIZE                                             5     DOT_PRODUCT                                             6     SUM                                             7     DPROD                                             8     MAX                                            9                                                      :                   	               &                                                                                      ;                   	               &                                                                                     <                   	               &                                                                                      =                   	               &                                           #         @                                  >                    #CGSTEP_CLOSE%ALLOCATED ?                                               ?     ALLOCATED #         @                                   @                    #DD A   #AA B   #NITER C            D P                               A                   	               &                                                     D @                               B     �               #FILTER              
  @                               C              �         fn#fn    �   @   J   HCONEST    �   @   J   CGSTEP_MOD    4  @   J   SOLVER_SMP_MOD    t  k       FILTER+HELIX !   �  �   a   FILTER%FLT+HELIX !   s  �   a   FILTER%LAG+HELIX !     �   a   FILTER%MIS+HELIX %   �  ]       HCONEST_INIT+HCONEST *   �  �   a   HCONEST_INIT%X_IN+HCONEST +   �  T   a   HCONEST_INIT%AA_IN+HCONEST *   �        SOLVER_SMP+SOLVER_SMP_MOD /   �  =      SOLVER_SMP%SIZE+SOLVER_SMP_MOD 2      @      SOLVER_SMP%PRESENT+SOLVER_SMP_MOD 6   `  D      SOLVER_SMP%DOT_PRODUCT+SOLVER_SMP_MOD ,   �  �   a   SOLVER_SMP%M+SOLVER_SMP_MOD ,   0  �   a   SOLVER_SMP%D+SOLVER_SMP_MOD .   �  p      SOLVER_SMP%FOP+SOLVER_SMP_MOD 2   ,  @   a   SOLVER_SMP%FOP%ADJ+SOLVER_SMP_MOD 2   l  @   a   SOLVER_SMP%FOP%ADD+SOLVER_SMP_MOD 0   �  �   a   SOLVER_SMP%FOP%M+SOLVER_SMP_MOD 0   8	  �   a   SOLVER_SMP%FOP%D+SOLVER_SMP_MOD 2   �	  z      SOLVER_SMP%STEPPER+SOLVER_SMP_MOD 9   >
  @   a   SOLVER_SMP%STEPPER%FORGET+SOLVER_SMP_MOD 4   ~
  �   a   SOLVER_SMP%STEPPER%M+SOLVER_SMP_MOD 4   
  �   a   SOLVER_SMP%STEPPER%G+SOLVER_SMP_MOD 5   �  �   a   SOLVER_SMP%STEPPER%RR+SOLVER_SMP_MOD 5   "  �   a   SOLVER_SMP%STEPPER%GG+SOLVER_SMP_MOD 0   �  @   a   SOLVER_SMP%NITER+SOLVER_SMP_MOD .   �  p      SOLVER_SMP%WOP+SOLVER_SMP_MOD 2   ^  @   a   SOLVER_SMP%WOP%ADJ+SOLVER_SMP_MOD 2   �  @   a   SOLVER_SMP%WOP%ADD+SOLVER_SMP_MOD 0   �  �   a   SOLVER_SMP%WOP%M+SOLVER_SMP_MOD 0   j  �   a   SOLVER_SMP%WOP%D+SOLVER_SMP_MOD .   �  p      SOLVER_SMP%JOP+SOLVER_SMP_MOD 2   f  @   a   SOLVER_SMP%JOP%ADJ+SOLVER_SMP_MOD 2   �  @   a   SOLVER_SMP%JOP%ADD+SOLVER_SMP_MOD 0   �  �   a   SOLVER_SMP%JOP%M+SOLVER_SMP_MOD 0   r  �   a   SOLVER_SMP%JOP%D+SOLVER_SMP_MOD -   �  �   a   SOLVER_SMP%M0+SOLVER_SMP_MOD .   �  �   a   SOLVER_SMP%ERR+SOLVER_SMP_MOD /     �   a   SOLVER_SMP%RESD+SOLVER_SMP_MOD /   �  �   a   SOLVER_SMP%MMOV+SOLVER_SMP_MOD /   F  �   a   SOLVER_SMP%RMOV+SOLVER_SMP_MOD /   �  @   a   SOLVER_SMP%VERB+SOLVER_SMP_MOD $   *  p       HCONEST_LOP+HCONEST (   �  @   a   HCONEST_LOP%ADJ+HCONEST (   �  @   a   HCONEST_LOP%ADD+HCONEST &     �   a   HCONEST_LOP%A+HCONEST &   �  �   a   HCONEST_LOP%Y+HCONEST "   2  �       CGSTEP+CGSTEP_MOD ,     B      CGSTEP%ALLOCATED+CGSTEP_MOD '   _  =      CGSTEP%SIZE+CGSTEP_MOD .   �  D      CGSTEP%DOT_PRODUCT+CGSTEP_MOD &   �  <      CGSTEP%SUM+CGSTEP_MOD (     >      CGSTEP%DPROD+CGSTEP_MOD &   Z  <      CGSTEP%MAX+CGSTEP_MOD )   �  @   a   CGSTEP%FORGET+CGSTEP_MOD $   �  �   a   CGSTEP%X+CGSTEP_MOD $   b  �   a   CGSTEP%G+CGSTEP_MOD %   �  �   a   CGSTEP%RR+CGSTEP_MOD %   z  �   a   CGSTEP%GG+CGSTEP_MOD (     d       CGSTEP_CLOSE+CGSTEP_MOD 2   j  B      CGSTEP_CLOSE%ALLOCATED+CGSTEP_MOD    �  c       FIND_PEF      �   a   FIND_PEF%DD    �  T   a   FIND_PEF%AA    �  @   a   FIND_PEF%NITER 