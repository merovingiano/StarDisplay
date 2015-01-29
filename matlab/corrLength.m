function [Cl, L, N] = corrLength(X, i)

Cl = [];
L = [];
N = [];
Xr = X(:,i);
s = size(Xr);
for n=1:s(1)
    k = find(Xr(n,:) < 0, 1, 'first');
    if not(isempty(k))
        if (k > 1)
            x0 = k - 1; x1 = k;
            y0 = Xr(n,k-1); y1 = Xr(n,k);
            b = (y1 - y0) / (x1 - x0);
            a = y1 - b * x1;
            x = -a/b;
            Cl = [Cl x];
            L = [L X(n,3)];
            N = [N X(n,1)];
        end
    end
end
Cl = [Cl zeros(1,1000)];
L = [L zeros(1,1000)];
N = [N ones(1,1000)];

