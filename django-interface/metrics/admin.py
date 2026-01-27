from django.contrib import admin
from .models import Mesure

class MesureAdmin(admin.ModelAdmin): # Renommé pour la cohérence
    list_display = ('id', 'dist1', 'timestamp') # Ajout du timestamp pour voir l'heure
    
admin.site.register(Mesure, MesureAdmin) # On enregistre Mesure, pas IoT