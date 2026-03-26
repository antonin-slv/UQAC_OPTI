# Projet 1 - Optimisation Avancée

## Introduction

Ce projet s'inscrit dans le cadre du cours d'optimisation avancée.


## notes : 

-> Dans matlab, on a fait en sorte d'inclure tout ce qu'il faut pour permettre de traiter les données du excel, et de modéliser la puissance de la machine à partir de La différence d'élévation et du débit d'eau.




# fits from matlab 


## Fit Name: Elav / Q aval
-> donne le ratio entre la hauteur en aval et le débit d'eau.
Logarithmic Curve Fit (log)
f(x) = a*log(x) + b

Coefficients and 95% Confidence Bounds
    Value   Lower   Upper 
a    2.9449    2.9396    2.9502
b    84.867    84.832    84.901

Goodness of Fit
           Value 
SSE         1025.6
R-square    0.9646
DFE          43822
Adj R-sq    0.9646
RMSE         0.153

## Fit Name: P/D/N ALL1
-> donne le ratio Puissance (f(x,y)) / débit (x) / Hauteur de chute nette (y); (pour les points de 130 m3/s et moins)

Polynomial Surface Fit (poly11)
f(x,y) = p00 + p10*x + p01*y

Coefficients and 95% Confidence Bounds
       Value    Lower    Upper 
p00    -45.427    -46.223    -44.631
p10     0.3916     0.3902     0.3931
p01     1.0177     0.9951     1.0402

Goodness of Fit
           Value 
SSE         6334.7
R-square    0.9499
DFE          14514
Adj R-sq    0.9499
RMSE        0.6606

## Fit Name: P/D/N ALL2
(Pour les points de 120 m3/s et plus)
Polynomial Surface Fit (poly22)
f(x,y) = p00 + p10*x + p01*y + p20*x^2 + p11*x*y + p02*y^2

Coefficients and 95% Confidence Bounds
       Value    Lower    Upper 
p00    -152.19    -166.19    -138.19
p10     0.6368      0.588     0.6855
p01     6.3523     5.7099     6.9946
p20    -0.0021    -0.0021     -0.002
p11     0.0068     0.0057     0.0079
p02    -0.0878    -0.0953    -0.0803

Goodness of Fit
             Value   
SSE         3.2883e+05
R-square        0.8004
DFE         1.8471e+05
Adj R-sq        0.8004
RMSE            1.3342

## NOMAD

Commande de base pour optimiser un prog : 
```
"%NOMAD_HOME%\bin\nomad.exe" param.txt
```