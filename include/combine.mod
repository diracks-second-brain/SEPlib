	  �  /   k820309    h          14.0        R��W                                                                                                           
       combine.f90 COMBINE                      @                             
                            @                             
                                                          
       #         @                                                     #GAUSS_INIT%ALLOCATED    #SIZE                                                     ALLOCATED           
                                             #         @                                                      #NPATCH_IN    #NWALL_IN 	   #NWIND_IN 
                                                                              &                                                                                    	                                  &                                                                                    
                                  &                                           %         @                                                           #ADJ    #ADD    #WALL    #WIND              
                                                       
                                                                                                           	               &                                                                                                         	               &                                           %         @                                                          #GAUSS_SOLVE%SPREAD    #GAUSS_SOLVE%MAXLOC    #GAUSS_SOLVE%ABS    #GAUSS_SOLVE%EPSILON    #GAUSS_SOLVE%SUM    #A    #B    #X                                                     SPREAD                                                  MAXLOC                                                  ABS                                                  EPSILON                                                  SUM           
                                                     	              &                   &                                                     
                                                     	              &                                                                                                         	               &                                           #         @                                                       #         @                                                     #WALLWTN%PRODUCT    #N    #NWALL    #NWIND    #WINDWT    #WALLWT                                                      PRODUCT                                                                            &                                                                                                                      &                                                                                                                      &                                                     
                                                     	              &                                                                                                          	               &                                           #         @                                  !                    #GAUSS_CLOSE%ALLOCATED "                                               "     ALLOCATED #         @                                   #                   #COMBINEN%DOT_PRODUCT $   #COMBINEN%PRODUCT %   #COMBINEN%SIZE &   #DATA '   #DATA0 (   #COMB )   #X *   #N +   #NWALL ,   #NWIND -   #WIND .                                              $     DOT_PRODUCT                                            %     PRODUCT                                            &     SIZE           
@@                               '                   	              &                   &                                                     
@ @                               (                   	              &                                                  0  D@                               )                   	               &                                                     D @                               *                   	               &                   &                                                    D P                               +                                  &                                                    D P                               ,                                  &                                                    D P                               -                                  &                                                     
  @                               .                   	              &                                              �         fn#fn    �   @   J   PATCH    �   @   J   GAUSS    <  @   J   MKWALLWT !   |  l       GAUSS_INIT+GAUSS +   �  B      GAUSS_INIT%ALLOCATED+GAUSS &   *  @   a   GAUSS_INIT%SIZE+GAUSS !   j  s       PATCH_INIT+PATCH +   �  �   a   PATCH_INIT%NPATCH_IN+PATCH *   i  �   a   PATCH_INIT%NWALL_IN+PATCH *   �  �   a   PATCH_INIT%NWIND_IN+PATCH     �  v       PATCH_LOP+PATCH $   �  @   a   PATCH_LOP%ADJ+PATCH $   7  @   a   PATCH_LOP%ADD+PATCH %   w  �   a   PATCH_LOP%WALL+PATCH %     �   a   PATCH_LOP%WIND+PATCH "   �  �       GAUSS_SOLVE+GAUSS )   g  ?      GAUSS_SOLVE%SPREAD+GAUSS )   �  ?      GAUSS_SOLVE%MAXLOC+GAUSS &   �  <      GAUSS_SOLVE%ABS+GAUSS *   !  @      GAUSS_SOLVE%EPSILON+GAUSS &   a  <      GAUSS_SOLVE%SUM+GAUSS $   �  �   a   GAUSS_SOLVE%A+GAUSS $   A	  �   a   GAUSS_SOLVE%B+GAUSS $   �	  �   a   GAUSS_SOLVE%X+GAUSS "   Y
  H       PATCH_CLOSE+PATCH !   �
  �       WALLWTN+MKWALLWT )   3  @      WALLWTN%PRODUCT+MKWALLWT #   s  �   a   WALLWTN%N+MKWALLWT '   �  �   a   WALLWTN%NWALL+MKWALLWT '   �  �   a   WALLWTN%NWIND+MKWALLWT (     �   a   WALLWTN%WINDWT+MKWALLWT (   �  �   a   WALLWTN%WALLWT+MKWALLWT "   /  c       GAUSS_CLOSE+GAUSS ,   �  B      GAUSS_CLOSE%ALLOCATED+GAUSS    �  �       COMBINEN %   �  D      COMBINEN%DOT_PRODUCT !   �  @      COMBINEN%PRODUCT    0  =      COMBINEN%SIZE    m  �   a   COMBINEN%DATA      �   a   COMBINEN%DATA0    �  �   a   COMBINEN%COMB    )  �   a   COMBINEN%X    �  �   a   COMBINEN%N    Y  �   a   COMBINEN%NWALL    �  �   a   COMBINEN%NWIND    q  �   a   COMBINEN%WIND 