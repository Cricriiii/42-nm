# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Dockerfile                                         :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: cgajean <cgajean@student.42.fr>            +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/10/09 18:35:43 by cgajean           #+#    #+#              #
#    Updated: 2026/11/03 15:51:31 by cgajean          ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

ARG SOURCE="ft_nm"
ARG WORKSPACE="/${SOURCE}"
ARG VERSION_TAG="1.0"

# ---------------------------------------------------------------------------- #
# development environment                                                      #
# ---------------------------------------------------------------------------- #
FROM fedora:43 AS dev

ARG SOURCE
ARG WORKSPACE

RUN dnf install -y \
        gcc \
        glibc-devel.i686 \
        glibc-static.i686 \
        make \
        valgrind \
        diffutils \
        binutils \
        elfutils \
        patchelf \
        zsh \
        which \
        file \
        tree \
    && dnf clean all \
    && rm -rf /var/cache/dnf

WORKDIR ${WORKSPACE}

CMD ["/bin/zsh"]


# ---------------------------------------------------------------------------- #
# production environment                                                       #
# ---------------------------------------------------------------------------- #
FROM fedora:43 AS prod

ARG SOURCE
ARG WORKSPACE

COPY --from=dev ${WORKSPACE}/my_nm /usr/bin/my_nm

CMD ["/bin/bash"]
