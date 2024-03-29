<?php
/* Author: Jérémy Bobbio based on previous translation by magik_cypress */

$labels = [];
$labels['sauserprefs'] = 'Spam';
$labels['managespam'] = 'Gérer les paramètres de détection des spams';
$labels['sauserprefssettings'] = 'Préférences liées au spams';
$labels['spamgeneralsettings'] = 'Préférences principales';
$labels['spamthres'] = 'Seuil';
$labels['spamsubject'] = 'Étiqueter le sujet';
$labels['spamsubjectblank'] = '(laisser vide pour ne pas mettre d\'étiquette)';
$labels['spamlang'] = 'Langues des messages';
$labels['enabled'] = 'Actif';
$labels['disabled'] = 'Inactif';
$labels['headers'] = 'En-têtes du message';
$labels['foldheaders'] = 'Autoriser plusieurs lignes dans les en-têtes';
$labels['spamlevelstars'] = 'Utiliser l\'en-tête "Spam Level"';
$labels['spamlevelchar'] = 'Symbole représentant le niveau de spam';
$labels['spamtests'] = 'Services externes de détection';
$labels['userazor1'] = 'Utiliser Razor v1';
$labels['userazor2'] = 'Utiliser Razor v2';
$labels['usepyzor'] = 'Utiliser Pyzor';
$labels['usebayes'] = 'Utiliser le filtre bayésien';
$labels['usedcc'] = 'Utiliser DCC';
$labels['skiprblchecks'] = 'Utiliser les listes noires (RBL)';
$labels['spamreportsettings'] = 'Préférences du rapport';
$labels['spamreport0'] = 'Inclure le rapport dans les en-têtes du message original';
$labels['spamreport1'] = 'Inclure tout le message original en pièce-jointe du rapport';
$labels['spamreport2'] = 'Inclure le texte du message original en pièce-jointe du rapport';
$labels['spamaddressrules'] = 'Adresses particulières';
$labels['welcomelist_from'] = 'Accepter les messages de';
$labels['blocklist_from'] = 'Rejeter les messages de';
$labels['welcomelist_to'] = 'Accepter les messages envoyé à';
$labels['addrule'] = 'Ajouter';
$labels['rule'] = 'Règle';
$labels['importfromaddressbook'] = 'Importer le carnet d\'adresses';
$labels['deleteall'] = 'Tout supprimer';
$labels['autoscore'] = 'Auto';
$labels['defaultscore'] = 'Score par défaut';
$labels['otherscore'] = 'Autre (%s)';
$labels['saupusedefault'] = 'Rétablir les préférences par défaut';
$labels['langoptions'] = 'Options de langues';
$labels['bayes'] = 'Préférences du filtre bayésien';
$labels['purgingbayes'] = 'Suppression des données du filtre bayésien...';
$labels['purgebayes'] = 'Supprimer les données personnelles du filtre bayésien';
$labels['bayesautolearn'] = 'Activer l\'apprentissage automatique du filtre bayésien';
$labels['bayesautooptions'] = 'Options de l\'apprentissage automatique';
$labels['bayesnonspam'] = 'Seuil des messages légitimes';
$labels['bayesspam'] = 'Seuil des spams';
$labels['bayesrules'] = 'Utiliser le filtre bayésien pour la détection';
$labels['help'] = 'Aide';

$messages = [];
$messages['sauserprefchanged'] = 'Préférences modifiées avec succès';
$messages['sauserpreffailed'] = 'Erreur : impossible de modifier les préférences';
$messages['spamthresexp'] = 'Tout ce qui se trouve au dessus du seuil sera considéré comme spam. Augmenter le seuil va donc augmenter le nombre de spams qui ne seront pas détectés, mais réduira les risques de faux positifs.';
$messages['spamlangexp'] = 'Choisir toutes les langues dans lesquelles seront écrits les messages reçus. Tous les messages reçus qui seront écrits dans une langue qui n\'aura pas été choisie sera traité comme un spam possible.';
$messages['headersexp'] = 'SpamAssassin ajoute des en-têtes contenant des informations sur les traitements effectués. Les options suivantes permettent de configurer ces en-têtes.';
$messages['spamtestssexp'] = 'Certains tests utilisent des services Internet externes pour aider à identifier les spams. Choisir ceux qui doivent êtres utilisés :';
$messages['spamreport'] = 'Quand un message est identifié comme spam, un rapport avec tous les tests et scores est créé...';
$messages['autowelcomelist'] = 'Les adresses qui se trouvent dans le carnet d\'adresses sont automatiquement ajoutées dans la liste des exceptions.';
$messages['welcomelistexp'] = 'Une astérisque (*) peut être utilisé comme joker pour les traitements généraux. Par exemple : *example.com ou *.example.com.';
$messages['spamaddressexists'] = 'Une règle existe déjà pour cette addresse';
$messages['spamenteraddress'] = 'Entrer une adresse à ajouter';
$messages['spamaddresserror'] = 'Erreur : l\'adresse semble invalide';
$messages['spamaddressdelete'] = 'Faut-il vraiment supprimer cette règle ?';
$messages['spamaddressdeleteall'] = 'Faut-il vraiment supprimer toutes les règles ?';
$messages['noaddressrules'] = 'Aucune règle n\'a été trouvé.';
$messages['importingaddresses'] = 'Importation des adresses en cours...';
$messages['usedefaultconfirm'] = 'Faut-il vraiment rétablir les préférences par défaut ? Cela supprimera tous les changements, dont les réglages spécifiques.';
$messages['purgebayesexp'] = 'Supprimer les données personnelles du filtre bayésien apprises de vos courriels';
$messages['bayesnonspamexp'] = 'Les messages classés en-dessous de ce seuil seront utilisés pour améliorer la détection des messages légitimes.';
$messages['bayesspamexp'] = 'Les messages classés au-dessus de ce seuil seront utilisés pour améliorer la détection des spams.';
$messages['bayesautohelp'] = 'Quand l\'apprentissage automatique du filtre bayésien est activé, le système traitera les messages qui sembleront être ou ne pas être des spams afin d\'améliorer la détection de futurs messages.';
$messages['bayeshelp'] = 'Le filtre bayésien essaye d\'identifier les spams à partir des mots ou des séquences de caractères qui se trouvent dans les messages qui sont ou ne sont pas des spams.';
$messages['rblhelp'] = 'Les adresses des serveurs qui se trouvent dans les liens hypertextes d\'un message seront vérifiés à partir de plusieurs "listes noires" ou listes de sources connues de spam.';
$messages['dcchelp'] = 'Le DCC ou le "Distributed Checksum Clearinghouse" est un système de serveurs collectant et contant les signatures de millions de courriels chaque jour. Plus la signature d\'un message reçu est similaire à celle de la liste, plus il a de chances d\'être un spam.';
$messages['pyzhelp'] = 'Pyzor est un réseau collaboratif de systèmes visant à détecter et bloquer les spams à partir de condensé de messages. Ces condensés sont générés de plusieurs manières différentes afin d\'autoriser de petites différences dans le contenu des messages.';
$messages['raz2help'] = 'Vipul\'s Razor est un réseau collaboratif et distribué de détection et de filtrage de spam se basant sur des envois de spams par les utilisateurs. La détection est faite à partir de signatures qui détectent efficacement le contenu changeant des spams. Les rapports sont validés grâce à une réputation associée à chaque rapporteur.';
$messages['raz1help'] = 'Vipul\'s Razor est un réseau collaboratif et distribué de détection et de filtrage de spam se basant sur des envois de spams par les utilisateurs. La détection est faite à partir de signatures qui détectent efficacement le contenu changeant des spams. Les rapports sont validés grâce à une réputation associée à chaque rapporteur. La version 1 a été remplacé par la version 2 et n\'est donc plus supportée.';
$messages['levelhelp'] = 'L\'en-tête "Spam Level" peut être ajouté à un message pour indiquer le score de ce dernier en répetant un symbole plusieurs fois. Cela peut être utile pour le traitement ultérieur du message.';
$messages['foldhelp'] = 'Est-ce que les en-têtes ajoutés par SpamAssassin doivent revenir à la ligne ? En d\'autres termes, est-ce qu\'ils doivent êtres répartis sur plusieurs lignes au lieu de n\'être que sur seule ligne très longue. Lorsqu\'il y a un retour à la ligne, la suivante sera indenté afin de marquer la continuité avec la précédente.';
$messages['bayesruleshlp'] = 'Avant que le filtre bayésien puisse être utilisé, il doit effectivement être "entrainé" pour pouvoir correctement détecter les spams. Désactiver le filtre bayésien permet au système de continuer à apprendre automatiquement et manuellement des messages reçus, mais il ne sera pas utilisé pour la détection des spams en eux-mêmes.';
$messages['purgebayesconfirm'] = 'Faut-il vraiment supprimer toutes les données personnelles du filtre bayésien ?';
