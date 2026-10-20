"""Génère les effets sonores du jeu (assets/sounds/*.wav) par synthèse.

Aucun fichier son externe : tout est construit à partir de sinusoïdes, de dents de scie et de bruit.
Usage : python3 scripts/generate_sounds.py   (nécessite numpy)
"""

import os
import wave

import numpy as np

RATE = 22050
OUT_DIR = os.path.join(os.path.dirname(__file__), "..", "assets", "sounds")
rng = np.random.default_rng(443)


def time_axis(duration):
    return np.arange(int(duration * RATE)) / RATE


def saw(freq, t):
    phase = np.cumsum(np.broadcast_to(freq, t.shape)) / RATE
    return 2.0 * (phase - np.floor(phase + 0.5))


def sine(freq, t):
    phase = np.cumsum(np.broadcast_to(freq, t.shape)) / RATE
    return np.sin(2 * np.pi * phase)


def low_pass(x, cutoff):
    """Filtre passe-bas du premier ordre, fréquence de coupure éventuellement variable."""
    cutoff = np.broadcast_to(cutoff, x.shape)
    alpha = 1 - np.exp(-2 * np.pi * cutoff / RATE)
    y = np.zeros_like(x)
    acc = 0.0
    for i in range(len(x)):
        acc += alpha[i] * (x[i] - acc)
        y[i] = acc
    return y


def envelope(t, attack, decay):
    return np.minimum(t / attack, 1.0) * np.exp(-t / decay)


def save(name, x, peak=0.8):
    x = peak * x / np.max(np.abs(x))
    data = (x * 32767).astype(np.int16)
    with wave.open(os.path.join(OUT_DIR, name), "wb") as f:
        f.setnchannels(1)
        f.setsampwidth(2)
        f.setframerate(RATE)
        f.writeframes(data.tobytes())
    print(f"{name}: {len(x) / RATE:.2f} s")


def engine():
    # Bourdonnement électrique, joué en boucle : toutes les fréquences font un nombre entier
    # de périodes en 1 s pour que la boucle soit sans raccord. La hauteur suit la vitesse dans le jeu.
    # On filtre deux périodes et on garde la seconde, une fois le filtre en régime établi.
    t = time_axis(2.0)
    x = 0.6 * saw(55, t) + 0.35 * saw(110, t) + 0.25 * sine(220, t) + 0.12 * sine(330, t)
    x *= 1 + 0.15 * np.sin(2 * np.pi * 6 * t)  # léger tremblement
    x = low_pass(x, 1800)
    return x[RATE:]


def explosion():
    t = time_axis(1.6)
    noise = rng.uniform(-1, 1, len(t))
    crackle = low_pass(noise, 4000 * np.exp(-t / 0.25) + 300) * envelope(t, 0.005, 0.35)
    thump = sine(70 * np.exp(-t / 0.4) + 25, t) * envelope(t, 0.005, 0.25)
    return crackle + 0.9 * thump


def boost():
    t = time_axis(0.7)
    freq = 300 * 2 ** (t / 0.7 * 2.2)  # montée de plus de deux octaves
    x = sine(freq, t) + 0.4 * sine(2 * freq, t) + 0.2 * saw(freq / 2, t)
    return x * envelope(t, 0.02, 0.35)


def fall():
    t = time_axis(1.6)
    freq = 900 * 2 ** (-t / 1.6 * 2.5)  # sifflement qui descend
    return (sine(freq, t) + 0.3 * sine(1.5 * freq, t)) * envelope(t, 0.05, 0.8)


def beep(freq, duration):
    t = time_axis(duration)
    x = sine(freq, t) + 0.3 * sine(2 * freq, t)
    return x * envelope(t, 0.005, duration / 3)


if __name__ == "__main__":
    os.makedirs(OUT_DIR, exist_ok=True)
    save("engine.wav", engine(), peak=0.6)
    save("explosion.wav", explosion(), peak=0.9)
    save("boost.wav", boost(), peak=0.7)
    save("fall.wav", fall(), peak=0.6)
    save("countdown.wav", beep(660, 0.18), peak=0.6)
    save("go.wav", beep(1320, 0.45), peak=0.7)
