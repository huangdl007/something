I1 = imread('sin1.gif');
I2 = imread('6.png');
I3 = imread('3.png');
I4 = imread('4.png');

subplot(4,2,1);imshow(I1);
fftI1 = fft2(I1);
sfftI1 = fftshift(fftI1);
RR1 = real(sfftI1);
II1 = imag(sfftI1);
A1 = sqrt(RR1.^2 + II1.^2);
A1 = (A1-min(min(A1))) / (max(max(A1))-min(min(A1))) * 255;
subplot(4,2,2);imshow(A1);

subplot(4,2,3);imshow(I2);
fftI2 = fft2(I2);
sfftI2 = fftshift(fftI2);
RR2 = real(sfftI2);
II2 = imag(sfftI2);
A2 = sqrt(RR2.^2 + II2.^2);
A2 = (A2-min(min(min(A2)))) / (max(max(max(A2)))-min(min(min(A2)))) * 255;
subplot(4,2,4);imshow(A2);

subplot(4,2,5);imshow(I3);
fftI3 = fft2(I3);
sfftI3 = fftshift(fftI3);
RR3 = real(sfftI3);
II3 = imag(sfftI3);
A3 = sqrt(RR3.^2 + II3.^2);
A3 = (A3-min(min(min(A3)))) / (max(max(max(A3)))-min(min(min(A3)))) * 255;
subplot(4,2,6);imshow(A3);

subplot(4,2,7);imshow(I4);
fftI4 = fft2(I4);
sfftI4 = fftshift(fftI4);
RR4 = real(sfftI4);
II4 = imag(sfftI4);
A4 = sqrt(RR4.^2 + II4.^2);
A4 = (A4-min(min(min(A4)))) / (max(max(max(A4)))-min(min(min(A4)))) * 255;
subplot(4,2,8);imshow(A4);