function [p_tot] = calculate_p_tot(HydroTable)
    % HydroTable est une feuille excell avec des données de prod.
    % On extrait les données en tant que vecteurs numériques
    cols = ["P1_MW_", "P2_MW_", "P3_MW_", "P4_MW_", "P5_MW_"];
    
    % On extrait ces colonnes en matrice et on somme sur la dimension 2 (les lignes)
    p_tot = sum(HydroTable{:, cols}, 2);
end


opts = detectImportOptions(filename);
TableBrute = readtable(filename, opts);

%% 2. Nettoyage global (Valeurs aberrantes)
% On retire les lignes où les niveaux sont physiquement impossibles (ex: 0 ou négatif)
cleanIdx = TableBrute.NivAmont_m_ > 0 & TableBrute.Elav_m_ > 0;
data = TableBrute(cleanIdx, :);
fprintf("gardées : %d lignes sur %d\n",length(data.Elav_m_), length(TableBrute.Elav_m_))

%% 3. Calcul de l'élévation avale (Modèle 1)
% Le débit total qui influence le niveau aval est la somme du turbiné et du déversé
% Selon vos colonnes : Q_total_aval = Qtot + Qvan

Q_aval = data.Qtot_m3_s_ + data.Qvan_m3_s_;
Elav = data.Elav_m_;

% Sauvegardez ces variables pour le cftool (Modèle Elav = f(Q_aval))
save('Data_Aval.mat', 'Q_aval', 'Elav');

%% 4. Pré-traitement par Turbine (Modèle 2 : Puissance)
% On va créer une structure pour stocker les données propres de chaque turbine
turbines = struct();

for i = 1:5
    % Noms dynamiques des colonnes
    Q_col = sprintf('Q%d_m3_s_', i);
    P_col = sprintf('P%d_MW_', i);
    
    % Extraction des données
    Qi = data.(Q_col);
    Pi = data.(P_col);
    H_amont = data.NivAmont_m_;
    H_aval = data.Elav_m_;
    
    % --- FILTRAGE CRITIQUE ---
    % On ne garde que si la turbine produit (P > 0.5 MW pour éviter le bruit)
    % et si le débit est significatif.
    idxActive = (Qi > 0) & (Pi > 0); 
    
    Q_active = Qi(idxActive);
    P_active = Pi(idxActive);
    H_net_active = (H_amont(idxActive) - H_aval(idxActive)) - (0.5e-5 * Q_active.^2);
    
    % Stockage des résultats
    turbines(i).Q = Q_active;
    turbines(i).Hn = H_net_active;
    turbines(i).P = P_active;
    
    fprintf('Turbine %d : %d points valides conservés.\n', i, length(P_active));
end

%% 5. Exportation pour le Curve Fitting
% Exemple pour la Turbine 1 (à répéter pour les autres)
Q_all = [];
Hn_all = [];
P_all = [];

for i = 1:5
    % On réutilise les données filtrées du bloc précédent
    Q_all = [Q_all ; turbines(i).Q];
    Hn_all = [Hn_all ; turbines(i).Hn];
    P_all = [P_all ; turbines(i).P];
end