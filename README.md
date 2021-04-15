# mini_projet_robotique
Membres de l'équipe : Balthazar Bujard (284179) - Microtechnique BA6
                      Juliette Challot (283171) - Systèmes de communication MA2 (mineur spatial)

Description du projet : nous voulons que notre e-puck traverse un labyrinthe sans heurter les murs et en suivant un code couleur quand plusieurs chemins s'offrent à lui. 

Etapes : 1- Faire avancer le robot le long d'un mur 
         2- Detecter la presence d'un mur à gauche, devant et à droite de l'e-puck 
         3- Si 1 mur : effectuer une rotation du robot vers le mur et activer la camera pour voir la couleur du mur et ainsi choisir ou aller par la suite : aller à gauche si le mur est de couleur rouge et aller à droite si il est bleu
         4- Si 2 murs : continuer dans la direction non murée
         5- Si 3 murs : ouvrir la camera pour vérifier que le mur est vert, si c'est le cas allumer les led pour signifier que le défi de gagné.



