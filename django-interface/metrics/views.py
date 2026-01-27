import socket
from django.http import JsonResponse, HttpResponse
from django.shortcuts import render
from .models import Mesure

STM32_IP = "192.168.1.15" 
STM32_PORT = 5000

def metrics(request):
    return render(request, 'index.html')

def data(request):
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
            s.settimeout(2)
            s.connect((STM32_IP, STM32_PORT))
            raw_val = s.recv(1024).decode().strip()
            
            dist = round((float(raw_val) * 0.0343) / 2, 2)
            
            obj = Mesure.objects.create(dist1=dist)
            return JsonResponse({'id': obj.id, 'dist1': dist})
    except Exception as e:
        return JsonResponse({'error': str(e)}, status=500)

def clear_data(request):
    Mesure.objects.all().delete()
    return HttpResponse('Données supprimées')