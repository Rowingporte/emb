from django.db import models

class Mesure(models.Model):
    dist1 = models.FloatField()
    timestamp = models.DateTimeField(auto_now_add=True) 

    def __str__(self):
        return f"Distance: {self.dist1} cm à {self.timestamp}"