figure('PaperType', 'a4', 'Units', 'centimeters', 'Position', [2, 0.0, 29.0, 29.7]);
n = size(P);
n = n(2);
tc = find(dist == min(dist));
I = 1:tc(1);
T = T(I);
lock = lock(I);
bank = bank(I);
dist = dist(I);
speed = speed(I);
Rp = Rp(I,:);
for i=1:n
    P{i} = P{i}(I,:);
    R{i} = R{i}(I,:);
end
n = size(P);
n = n(2);
l = find(lock >= 0);
l = lock(l(end));
t2 = find(T > T(end)-2);
colors = {'blue', 'green', 'cyan', 'magenta', 'black'};
markers = {'+','o','x','s','d'};

subplot(6,2,10);
plot(T-T(1), Rp(:,2),'black', 'LineWidth', 1);
title('\bfPredator altitude');
axis tight;
ylabel 'Altitude [m]';
hold on;
plot(T(t2(1))-T(1), Rp(t2(1),2), 'sblack', 'MarkerFaceColor', 'r');
plot(T(t2(end))-T(1), Rp(t2(end),2), 'oblack', 'MarkerFaceColor', 'r');

subplot(6,2,8)
plot(T-T(1), -bank,'black', 'LineWidth', 1);
title('\bfPredator banking angle');
axis tight;
ylabel 'Bank [deg]';
hold on;
plot(T(t2(1))-T(1), -bank(t2(1)), 'sblack', 'MarkerFaceColor', 'r');
plot(T(t2(end))-T(1), -bank(t2(end)), 'oblack', 'MarkerFaceColor', 'r');

subplot(6,2,12)
plot(T-T(1), dist,'black', 'LineWidth', 1);
title('\bfPredator-Prey distance');
axis tight;
xlabel 'Time [s]';
ylabel 'Distance [m]';
hold on;
plot(T(t2(1))-T(1), dist(t2(1)), 'sblack', 'MarkerFaceColor', 'r');
plot(T(t2(end))-T(1), dist(t2(end)), 'oblack', 'MarkerFaceColor', 'r');

subplot(6,2,[7 9 11]);
hold on;
for i=1:n
    plot(R{i}(:,1), -R{i}(:,3), colors{i}, 'LineWidth', 1);
end
plot(Rp(:,1), -Rp(:,3), 'r', 'LineWidth', 1);
title('\bfTrajectories in xy-plane (red: predator)');
axis equal
xlabel 'x [m]'
ylabel 'y [m]'
plot(Rp(t2(1),1), -Rp(t2(1),3), 'sblack', 'MarkerFaceColor', 'r');
plot(Rp(t2(end),1), -Rp(t2(end),3), 'oblack', 'MarkerFaceColor', 'r');

subplot(6,2,[1 3 5]);
hold on
for i=1:n
    t = find(P{i}(:,3) < 1);
    plot(720 + 720 * P{i}(t,1),360 + 360 * P{i}(t,2), colors{i}, 'Marker', markers{i});
end
title('\bfOversized CCD');
axis equal;
xlabel 'x [pixel]'
ylabel 'y [pixel]'
plot(720 + 720 * P{l}(t2(1),1), 360 + 360 * P{l}(t2(1),2), 'sblack', 'MarkerFaceColor', 'r');
plot(720 + 720 * P{l}(t2(end),1), 360 + 360 * P{l}(t2(end),2), 'oblack', 'MarkerFaceColor', 'r');

subplot(6,2,[2 4 6]);
hold on;
for i=1:n
    plot(720 + 720 * P{i}(t,1),360 + 360 * P{i}(t,2),colors{i}, 'Marker', markers{i});
end
axis equal;
title('\bfRealistic CCD');
axis( [0 1024 0 720] );
plot([0 1024],[360 360],'--black');
plot([512 512],[0 720],'--black');
xlabel 'x [pixel]'
ylabel 'y [pixel]'
plot(720 + 720 * P{l}(t2(1),1), 360 + 360 * P{l}(t2(1),2), 'sblack', 'MarkerFaceColor', 'r');
plot(720 + 720 * P{l}(t2(end),1), 360 + 360 * P{l}(t2(end),2), 'oblack', 'MarkerFaceColor', 'r');

set(gcf, 'Position', [2, 0.0, 25.0, 29.7]);

