from django.urls import path
from . import views

urlpatterns = [
    path('metrics/', views.metrics, name='metrics'),
    path('data/', views.data, name='data'),
    path('clear_data/', views.clear_data, name='clear_data'),

]
