import pygame
import os
import time
import threading
import random

# =======================
# Configuration
# =======================

ZONE_NAMES = ['HA', 'MF', 'CF', 'LF', 'LM', 'HF']
EXPECTED_PRESSURE = {
    'HA': 0.10,
    'MF': 0.19,
    'CF': 0.18,
    'LF': 0.17,
    'LM': 0.04,
    'HF': 0.32,
}
ZONE_CONFIG = {
    'HA': (249, 54),
    'MF': (275, 198),
    'CF': (177, 214),
    'LF': (103, 261),
    'LM': (137, 399),
    'HF': (228, 573),
}

FPS = 30
RADIUS = 20

# Image path
ASSET_PATH = os.path.join(os.path.dirname(__file__), "assets", "foot.png")

# =======================
# Color Helpers
# =======================

def pressure_to_color(actual, expected):
    if actual is None:
        return (255, 255, 0)  # yellow
    diff = actual - expected
    threshold = 0.10
    if abs(diff) < threshold:
        return (255, 255, 255)  # white
    elif diff > 0:
        return (255, 0, 0)  # red
    else:
        return (0, 0, 255)  # blue

# =======================
# Warning Symbol
# =======================

def draw_warning_triangle(screen, center):
    x, y = center
    points = [(x, y - 10), (x - 10, y + 10), (x + 10, y + 10)]
    pygame.draw.polygon(screen, (255, 255, 0), points)

# =======================
# Mock Pressure Provider (Replace in real use)
# =======================

def mock_get_pressure():
    return [random.uniform(0, 0.35) if random.random() > 0.05 else None for _ in range(6)]

# =======================
# Main Runner
# =======================

def run_pressure_gui(get_pressure_fn):
    pygame.init()
    screen = pygame.display.set_mode((400, 600))
    pygame.display.set_caption("Foot Pressure GUI")

    # Load foot image
    try:
        foot_img = pygame.image.load(ASSET_PATH).convert()
        foot_img = pygame.transform.scale(foot_img, (400, 650))  # Scale to window size
    except Exception as e:
        print(f"Failed to load image: {ASSET_PATH}")
        raise e

    clock = pygame.time.Clock()
    font = pygame.font.SysFont(None, 20)

    running = True
    while running:
        screen.fill((0, 0, 0))
        screen.blit(foot_img, (0, 0))

        for event in pygame.event.get():
            if event.type == pygame.QUIT:
                running = False

            elif event.type == pygame.MOUSEBUTTONDOWN:
                x, y = pygame.mouse.get_pos()
                print(f"Clicked at: ({x}, {y})")

        # Get and draw pressure
        pressures = get_pressure_fn()
        for i, zone in enumerate(ZONE_NAMES):
            x, y = ZONE_CONFIG[zone]
            value = pressures[i]
            color = pressure_to_color(value, EXPECTED_PRESSURE[zone])
            pygame.draw.circle(screen, color, (x, y), RADIUS)
            label = font.render(zone, True, (0, 0, 0))
            screen.blit(label, (x - 10, y + RADIUS + 5))
            if value is None:
                draw_warning_triangle(screen, (x, y))

        pygame.display.flip()
        clock.tick(FPS)

    pygame.quit()
