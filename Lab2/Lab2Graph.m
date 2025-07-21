u = [49.4226,79.4785,110.7211,160.079193];
var = [15.144369,21.492956,26.886707,35.986778];
plot(u,var,'x'),xlabel('Mean'),ylabel('Variance'),title('Mean & Variance w/ Line of Best Fit'), axis([0 200 0 40]);
hold
m = 0.1865719;
b = 6.23444065;
x = 0:0.01:200;
y = m*x + b;
plot(x,y);