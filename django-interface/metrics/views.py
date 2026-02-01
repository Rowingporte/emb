import socket
from django.http import JsonResponse, HttpResponse
from django.shortcuts import render
from .models import Mesure

LISTEN_IP = "0.0.0.0" 
UDP_PORT = 5000

def metrics(request):
    return render(request, 'index.html')

def data(request):
    try:
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
            s.settimeout(1.0)
            
            s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            s.bind((LISTEN_IP, UDP_PORT))
            
  
            packet, addr = s.recvfrom(1024)
            raw_val = packet.decode().strip()
            
            try:
                numeric_part = "".join(c for c in raw_val if c.isdigit() or c == '.')
                dist = float(numeric_part)
            except ValueError:
                return JsonResponse({'error': f'Invalid data format: {raw_val}'}, status=500)
            
            obj = Mesure.objects.create(dist1=dist)
            return JsonResponse({'id': obj.id, 'dist1': dist, 'from': addr[0]})

    except socket.timeout:
        return JsonResponse({'error': 'Timeout: No data received from sensor'}, status=504)
    except Exception as e:
        return JsonResponse({'error': str(e)}, status=500)

def clear_data(request):
    Mesure.objects.all().delete()
    return HttpResponse('Données supprimées')