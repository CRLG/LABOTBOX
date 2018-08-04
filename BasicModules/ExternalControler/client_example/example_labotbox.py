#!/usr/bin/env python
# coding: utf-8

import sys
sys.path.append('./client')
from labotbox_client_tcp import *
from labotbox_client_websocket import *
import datetime

connection_type = "tcp"  # Valeurs possibles : "tcp" ou "websocket"
host = "127.0.0.1"
port = 1234

# _______________________________________________
# Création d'une instance et connexion automatique avec le serveur
if (connection_type == "tcp"):
	client = LabotboxClientTcp(host, port)
elif (connection_type == "websocket"):
	client = LabotboxClientWebSocket(host, port)
else:
	print ("ERROR Connection is not definied: " + connection_type)

# _______________________________________________
# Active ou non le mode verbeux pour avoir un affichage de tous les messages envoyés/reçus avec le serveur
client.setDebug(False) 

# _______________________________________________
# Récupération auprès de Labotbox de l'API complète
print client.help()

# _______________________________________________
# Récupération de l'aide sur une instruction donnée
print client.helpInstruction('setData')

# _______________________________________________
# Envoie d'une commande lbre
client.sendCommand('setData(PosX_robot, 1977)');

# _______________________________________________
# Ecriture d'une donnée avec une chaine de caractère
client.setData('PosX_robot', 'Coucou la donnée')

# _______________________________________________
# Ecriture d'une donnée avec une valeur numérique venant d'une variable
a=879.356
client.setData('PosX_robot', a)

# _______________________________________________
# Récupération d'une donnée en forçant le type de retour à string
print client.getDataStr('PosX_robot') 

# _______________________________________________
# Récupération d'une donnée sachant qu'elle est de type numérique
# La fonction renverra une donnée de type int ou float suivant si la valeur est détectée sans ou avec décimales
# Si la conversion vers une donnée de type numérique n'est pas possible, une exception sera levée
pos_x_robot = client.getDataNum('PosX_robot')
print(pos_x_robot, type(pos_x_robot))

# _______________________________________________
# Gestion des codes d'erreurs sous forme d'exceptions
# Dès que Labotbox renvoie un code d'erreur différent de "0", une exception est levée
# Il est de la responsabilité de l'utilisateur de gérer ou non les exceptions
# S'il ne les gère pas et qu'une erreur est remontée, le script s'arrêtera
try:
	value = client.getDataStr('Dummy_Data_Name') # ici, volotairement, cette donnée "PosXY_robot" n'existe pas pour mettre en évidence la levée de l'exception
except ValueError as err:
    print('! There was an exception but it\'s normal !' + str(err))

# _______________________________________________
i=0.0
while i<3:
	client.setData('PosX_robot', i)
	print client.getDataNum('PosX_robot')
	i+=0.1

# _______________________________________________
# Récupère en une seule fois les valeurs de toutes les données
liste_data_value = client.getDatas()
i=0
while i<len(liste_data_value):
	dataname = liste_data_value[i][0]
	datavalue = liste_data_value[i][1]
	i += 1
	print(dataname + " = " + datavalue)

# _______________________________________________
# Créé une nouvelle variable dans le DataManager avec une valeur par défaut
str_datatime = str(datetime.datetime.now())  # Utilise la date et l'heure pour être certain que la data n'existe pas déjà
str_datatime = str_datatime.replace(" ", "_")
dataname = "fromPython_" + str_datatime
client.createData(dataname, 123.456)

# _______________________________________________
# Fin de la communication avec le serveur
client.close()

# _______________________________________________
# Ouvre une nouvelle connexion
print client.connect(host, port)
print client.help()
