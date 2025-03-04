#!/bin/bash
#fichier généré à l'aide de ChatGPT 4o

# Fonction pour décommenter une ligne spécifique dans un fichier
uncomment_specific_line() {
    local file="$1"
    local search_pattern="$2"
    local comment_symbol="$3"

    # Créer un fichier temporaire pour stocker le contenu modifié
    local temp_file=$(mktemp)

    # Lire chaque ligne du fichier
    while IFS= read -r line; do
        # Vérifier si la ligne contient le motif de recherche et est commentée
        if [[ "$line" == "$comment_symbol"*"$search_pattern"* ]]; then
            # Supprimer le symbole de commentaire
            echo "${line#$comment_symbol }" >> "$temp_file"
        else
            echo "$line" >> "$temp_file"
        fi
    done < "$file"

    # Remplacer le fichier original par le fichier temporaire
    mv "$temp_file" "$file"
}

# Motif de recherche pour la ligne à décommenter
search_pattern="define DIFT_H"

# Liste des fichiers à modifier
files_to_modify=(
    "$ROOT_DIR/ips/riscv/include/riscv_config.sv"
    "$ROOT_DIR/rtl/includes/config.sv"
    "$ROOT_DIR/sw/apps/buffer_overflow/buffer_overflow.c"
    "$ROOT_DIR/sw/apps/format_string/format_string.c"
    # Ajoute d'autres fichiers ici
)

# Parcourir chaque fichier et décommenter la ligne spécifique
for file in "${files_to_modify[@]}"; do
    if [[ -f "$file" ]]; then
        case "$file" in
            *.c|*.h)
                uncomment_specific_line "$file" "$search_pattern" "//"
                ;;
            *.v|*.sv)
                uncomment_specific_line "$file" "$search_pattern" "//"
                ;;
        esac
    else
        echo "Le fichier $file n'existe pas."
    fi
done

echo "Passage du processeur RI5CY au processeur D-RI5CY."