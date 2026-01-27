import socket
from django.http import JsonResponse, HttpResponse
from django.shortcuts import render
from .models import Mesure

# Use 0.0.0.0 to listen to the broadcast on all interfaces
LISTEN_IP = "0.0.0.0" 
UDP_PORT = 5000

def metrics(request):
    return render(request, 'index.html')

def data(request):
    try:
        # 1. Create a UDP socket (SOCK_DGRAM)
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as s:
            # Set a short timeout so the web page doesn't hang forever if sensor is off
            s.settimeout(1.0)
            
            # 2. Bind the socket to the port (Like socat does)
            # This allows us to "hear" the broadcast packets
            s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
            s.bind((LISTEN_IP, UDP_PORT))
            
            # 3. Receive the data
            packet, addr = s.recvfrom(1024)
            raw_val = packet.decode().strip()
            
            # Clean the string: if your C code sends "dist: 10.50 cm", 
            # we need to extract just the number.
            # If your C code only sends the raw number, this float() is enough.
            try:
                # Extract number if string is "dist: 12.34 cm"
                numeric_part = "".join(c for c in raw_val if c.isdigit() or c == '.')
                dist = float(numeric_part)
            except ValueError:
                return JsonResponse({'error': f'Invalid data format: {raw_val}'}, status=500)
            
            # 4. Save to DB
            obj = Mesure.objects.create(dist1=dist)
            return JsonResponse({'id': obj.id, 'dist1': dist, 'from': addr[0]})

    except socket.timeout:
        return JsonResponse({'error': 'Timeout: No data received from sensor'}, status=504)
    except Exception as e:
        return JsonResponse({'error': str(e)}, status=500)

def clear_data(request):
    Mesure.objects.all().delete()
    return HttpResponse('Données supprimées')