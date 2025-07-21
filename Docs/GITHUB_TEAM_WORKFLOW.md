# Using GitHub for Team Collaboration

## Why We Use GitHub
- **Version control:** Track all code changes
- **Collaboration:** Multiple team members work together
- **Backup:** Code is safe in the cloud
- **Code review:** Ensure quality before merging

## Project Repository Structure

Example (cmd/tree style):
```
iot-enviorment-monitoring/
├── Core/
│   ├── Drivers/
│   │   ├── bme680/
│   │   ├── lora/
│   │   └── usart/
│   ├── Src/
│   ├── Inc/
│   └── Startup/
├── Docs/
│   ├── WIRING_CONFIGURATION.md
│   └── ...
└── ...
```

## Branching Example
- Each feature or fix is developed in a separate branch
- Example branch: `feature/lora-cleanup`
- Purpose: Refactor LoRa driver for datasheet compliance
- After testing, branch is merged via Pull Request

## Pull Request and Commit Example
**Pull Request Workflow:**
1. Push branch to GitHub
2. Open a Pull Request (PR)
3. Team reviews code, suggests changes
4. Example commit message:
   ```
   Refactor: Update SX1262 LoRa TX sequence to match datasheet
   - Set standby, packet type, modulation, and IRQ as per spec
   - Add DIO1 TX_DONE handling
   - Remove debug prints
   ```
5. PR is approved and merged

## Documentation in the Project
- All documentation is kept in the `Docs/` folder
- Examples:
  - `WIRING_CONFIGURATION.md` -- hardware pinout
  - `SX1262_SPI_GUIDE.md` -- LoRa wiring and troubleshooting
  - `ProjectPresentation.tex` -- this presentation
- Documentation is updated with code changes
- Helps new team members onboard quickly 