from django.contrib import admin
from django.urls import path, include

urlpatterns = [
    path('admin/', admin.site.urls),
    path('', include('metrics.urls')), # On inclut les URLs de ton app metrics
]