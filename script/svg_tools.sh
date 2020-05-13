#!/bin/bash

SVG_SRC_DIR="${HOME}/src/WizNotePlus/share/skins/default"
SVG_SRC_DIR_MAC="${HOME}/src/WizNotePlus/share/skins/default/mac"
SVG_DEST_DIR="${HOME}/usr/WizNotePlus/share/skins/default"
SVG_DEST_DIR_MAC="${HOME}/usr/WizNotePlus/share/skins/default/mac"
#SVG_DEST_DIR="${HOME}/usr/WizNotePlus/tmp/skins/default"
#SVG_DEST_DIR_MAC="${HOME}/usr/WizNotePlus/tmp/skins/default/mac"

is_svg_file() {
    local file="$1"
    if [[ "${file}" =~ .*\.svg ]]; then
        return 0
    fi
    return 1
}

copy_all_svg_files() {
    local src="$1" dest="$2"
    mkdir -p "${dest}"
    local i=0
    for file in ${src}/*; do
        if is_svg_file "${file}"; then
            i=$((i+1))
            cp "${file}" "${dest}"
        fi
    done
    echo "Copied svg $i files to ${dest}"
}

change_svg_color() {
    local file="$1" color="$2"
    sed -En -i -e "s/fill=\"(#.{6})\"/fill=\"${color}\"/;p" "${file}"
}

# generate "selected" or "on" version of a svg icon, only if the old png icon
# has that derived version. This function only process the base version of svg 
# icon which has not the "selected" or "on" suffix.
generate_derived_svg() {
    local skin_dir=$1
    cd "${skin_dir}"
    for file in *; do
        if is_svg_file "${file}"; then
            if [[ "${file}" =~ .*_on\.svg ]] || \
                    [[ "${file}" =~ .*_selected\.svg ]]; then
                continue
            fi
            local basename="$(basename -s '.svg' ${file})"
            #rm "${basename}.png" "${basename}@2x.png"
            if [[ -f "${basename}_selected.png" ]]; then
                cp -f "${basename}.svg" "${basename}_selected.svg"
                change_svg_color "${basename}_selected.svg" "#448adf"
                #rm "${basename}_selected.png" "${basename}_selected@2x.png"
                echo "Derived selected icon: ${file}"
            fi
            if [[ -f "${basename}_on.png" ]]; then
                cp -f "${basename}.svg" "${basename}_on.svg"
                change_svg_color "${basename}_on.svg" "#448adf"
                #rm "${basename}_on.png" "${basename}_on@2x.png"
                echo "Derived on icon: ${file}"
            fi
        fi
    done
}

delete_old_png_files() {
    local skin_dir=$1
    cd "${skin_dir}"
    for file in *; do
        if is_svg_file "${file}"; then
            local basename="$(basename -s '.svg' ${file})"
            if rm "${basename}.png" "${basename}@2x.png"; then
                echo "Deleted ${basename}.png ${basename}@2x.png"
            else
                echo "Error: can not delete ${basename}.png" "${basename}@2x.png"
        fi
    done
}

change_category_selected_files() {
    local skin_dir=$1
    cd "${skin_dir}"
    for file in category_*_selected.svg; do
        change_svg_color "$file" "#FFFFFF"
    done
}

#copy_all_svg_files "${SVG_SRC_DIR}" "${SVG_DEST_DIR}"
#copy_all_svg_files "${SVG_SRC_DIR_MAC}" "${SVG_DEST_DIR_MAC}"
#generate_derived_svg "${SVG_DEST_DIR}"
#generate_derived_svg "${SVG_DEST_DIR_MAC}"
#delete_old_png_files "${SVG_DEST_DIR}"
#delete_old_png_files "${SVG_DEST_DIR_MAC}"
change_category_selected_files "${SVG_DEST_DIR}"