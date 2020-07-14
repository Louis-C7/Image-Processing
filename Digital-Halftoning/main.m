image = imread('peppers.tif');             % ¶ÁÈ¡Í¼Ïñ
img=double(image);                         % double»¯
out0 = Error_Diffusion(img,0, 0);             % Stucki filter(raster scan)
out1 = Error_Diffusion(img,0, 1);             % Floyd-Steinberg filter(raster scan)
out2 = Error_Diffusion(img,1, 0);             % Stucki filter(serpentine scan)
out3 = Error_Diffusion(img,1, 1);             % Floyd-Steinberg filter(serpentine scan)
figure(1);
subplot(2,3,[1,4])
imshow(image);title('Original image');
subplot(2,3,2)
imshow(out0);title('Stucki(raster scan)');
subplot(2,3,3)
imshow(out1);title('Floyd-Steinberg(raster scan)');
subplot(2,3,5)
imshow(out2);title('Stucki(serpentine scan)');
subplot(2,3,6)
imshow(out3);title('Floyd-Steinberg(serpentine scan)');